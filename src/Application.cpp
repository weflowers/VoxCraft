#include <iostream>
#include <GL/gl3w.h>
#include "Application.hpp"
#include "Resource.hpp"
#include "UI.hpp"

Application::Application() : width_(4000/3), height_(3000/3), in_game_(false)
{
	InitWindow();
}

void Application::InitWindow()
{
	glfwInit();

	//glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);

	window_ = glfwCreateWindow(width_, height_, "VoxCraft", nullptr, nullptr);

	if(window_ == nullptr)
	{
		printf("ERROR WHEN CREATING WINDOW");
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window_);

	gl3wInit();

	UI::Init(window_);
	Resource::InitResources();
	Setting::InitSetting();
}

void Application::Run()
{
	game_menu_ = std::make_unique<GameMenu>(window_);
	while(!glfwWindowShouldClose(window_))
	{
		if(in_game_)
		{
			world_controller_->Update();

			if(world_controller_->IsQuit())
			{
				world_controller_.reset();
				game_menu_ = std::make_unique<GameMenu>(window_);
				in_game_ = false;
			}
		}
		else
		{
			game_menu_->Update();
			if(game_menu_->EnterGame())
			{
				std::string worldName = game_menu_->GetWorldName();
				game_menu_.reset();
				world_controller_ = std::make_unique<WorldController>(window_, worldName);
				in_game_ = true;
			}
			else if(game_menu_->IsQuit()) //close game
				glfwSetWindowShouldClose(window_, true);
		}

		glfwSwapBuffers(window_);
		glfwPollEvents();
	}
}

Application::~Application()
{
	UI::Shutdown();
	Setting::SaveSetting();
	glfwDestroyWindow(window_);
}

