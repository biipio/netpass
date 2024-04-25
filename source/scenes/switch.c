#include "switch.h"
#include <stdlib.h>
#define N(x) scenes_switch_namespace_##x

void N(init)(Scene* sc) {
	sc->next_scene = ((Scene*(*)(void))sc->data)();
}
void N(render)(Scene* sc) { }
void N(exit)(Scene* sc) { }

SceneResult N(process)(Scene* sc) {
	return scene_switch;
}

Scene* getSwitchScene(Scene*(*next_scene)(void)) {
	Scene* scene = malloc(sizeof(Scene));
	if (!scene) return NULL;
	scene->init = N(init);
	scene->render = N(render);
	scene->exit = N(exit);
	scene->process = N(process);
	scene->data = (u32)next_scene;
	scene->need_free = true;
	return scene;
}