#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <glm/glm.hpp>

class World;

namespace Renderer
{
	extern void RenderWorld(const World &wld, const glm::mat4 &vp_matrix, const glm::vec3 &position,
								const glm::ivec3 &selection);
	extern void RenderCrosshair(const glm::mat4 &vp_matrix);
	extern void RenderSky(const glm::mat3 &view, const glm::mat4 &projection,
						  const glm::mat4 &sun_model_matrix, float day_time);

	extern void RenderMenuBg();
};


#endif //RENDERER_HPP
