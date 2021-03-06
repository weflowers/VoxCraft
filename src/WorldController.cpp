#include "Renderer.hpp"
#include "WorldController.hpp"
#include "UI.hpp"
#include <glm/gtx/string_cast.hpp>

void WorldController::FocusCallback(GLFWwindow *window, int focused)
{
	auto target = (WorldController*)glfwGetWindowUserPointer(window);
	if(!focused)
		target->control_ = false;
}
void WorldController::FramebufferSizeCallback(GLFWwindow *window, int width, int height)
{
	auto target = (WorldController*)glfwGetWindowUserPointer(window);
	target->Resize(width, height);
}
void WorldController::KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	auto target = (WorldController*)glfwGetWindowUserPointer(window);
	if(action == GLFW_PRESS)
	{
		if(key == GLFW_KEY_ESCAPE)
		{
			glfwSetCursorPos(window, target->width_ / 2, target->height_ / 2);
			target->control_ = !target->control_;
		}
		else if(key == GLFW_KEY_F)
			target->world_.GetPlayer().flying_ = !target->world_.GetPlayer().flying_;
		else if(key == GLFW_KEY_U)
			target->show_ui_ = !target->show_ui_;
	}
}
void WorldController::ScrollCallback(GLFWwindow *window, double x_offset, double y_offset)
{
	auto target = (WorldController*)glfwGetWindowUserPointer(window);

	auto y = (int)y_offset;

	uint8_t &usingBlock = target->world_.GetPlayer().using_block_;
	usingBlock -= y;
	if(usingBlock >= BLOCKS_NUM)
		usingBlock = 1;
	else if(usingBlock <= 0)
		usingBlock = BLOCKS_NUM - 1;
}

WorldController::WorldController(GLFWwindow *window, const std::string &world_name) :
		window_(window), world_(world_name), control_(true), show_ui_(true), is_quit_(false)
{
	glfwSetWindowUserPointer(window, (void*)this);

	glfwSetWindowFocusCallback(window, FocusCallback);
	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetScrollCallback(window, ScrollCallback);

	UI::CaptureEvent(window, false);

	int width, height;
	glfwGetFramebufferSize(window_, &width, &height);
	Resize(width, height);
}

WorldController::~WorldController()
{
	glfwSetWindowUserPointer(window_, nullptr);

	glfwSetWindowFocusCallback(window_, nullptr);
	glfwSetFramebufferSizeCallback(window_, nullptr);
	glfwSetKeyCallback(window_, nullptr);
	glfwSetScrollCallback(window_, nullptr);
}

void WorldController::Update()
{
	//logic process
	LogicProcess();
	//render
	Render();

	UI::NewFrame();
	RenderUI();
	UI::Render();
	glfwSetInputMode(window_, GLFW_CURSOR, control_ ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
}

void WorldController::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 view_matrix = world_.GetPlayer().GetViewMatrix();
	glm::mat4 vp_matrix = matrices_.GetProjection() * view_matrix;

	Renderer::RenderSky(glm::mat3(view_matrix), matrices_.GetProjection(),
						world_.GetSunModelMatrix(), world_.GetDayTime());
	Renderer::RenderWorld(world_, vp_matrix, world_.GetPlayer().position_, world_.GetPlayer().GetSelection(false));
	if(control_)
		Renderer::RenderCrosshair(matrices_.GetOrthoCenter());
}

void WorldController::LogicProcess()
{
	//update frame rate info
	framerate_.Update();
	world_.GetPlayer().Control(control_, window_, width_, height_, framerate_, matrices_.GetProjection());
	world_.Update(world_.GetPlayer().GetChunkPosition());
}

void WorldController::RenderUI()
{
	//information box
	if(control_)
	{
		if(show_ui_)
		{
			ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f));
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f)); // Transparent background
			if (ImGui::Begin("INFO", nullptr,
							 ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize
							 |ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings))
			{
				ImGui::Text("fps: %f", framerate_.GetFps());
				ImGui::Text("world info: (name: %s, seed: %d)", world_.GetName().c_str(), world_.GetSeed());
				ImGui::Text("running threads: %u", world_.GetRunningThreadNum());
				ImGui::Text("position: %s", glm::to_string(world_.GetPlayer().position_).c_str());
				ImGui::Text("chunk position: %s", glm::to_string(world_.GetPlayer().GetChunkPosition()).c_str());
				ImGui::Text("time: %f", world_.GetDayTime());
				ImGui::Text("flying [F]: %s", world_.GetPlayer().flying_ ? "true" : "false");
				ImGui::Text("using block: %s", BlockMethods::GetName(world_.GetPlayer().using_block_));

				ImGui::End();
			}
			ImGui::PopStyleColor();
		}
	}
	else //game menu
	{
		ImGui::SetNextWindowPosCenter();
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f)); // Transparent background
		if (ImGui::Begin("MENU", nullptr,
						 ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_AlwaysAutoResize
						 |ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoScrollbar))
		{
			ImGui::Text("Game menu");
			if(ImGui::Button("Continue [ESC]", ImVec2(500.0f, 0.0f)))
			{
				glfwSetCursorPos(window_, width_ / 2, height_ / 2);
				control_ = true;
			}
			if(ImGui::Button("Save and Back to menu", ImVec2(500.0f, 0.0f)))
				is_quit_ = true;

			ImGui::End();
		}
		ImGui::PopStyleColor();
	}
}

void WorldController::Resize(int width, int height)
{
	width_ = width;
	height_ = height;
	glViewport(0, 0, width, height);
	matrices_.UpdateProjection(width, height, WALK_FOVY);
	matrices_.UpdateOrtho(width, height);

	if(control_)
		glfwSetCursorPos(window_, width / 2, height / 2);
}

bool WorldController::IsQuit()
{
	return is_quit_;
}

