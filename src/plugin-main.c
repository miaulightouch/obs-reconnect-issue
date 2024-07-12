/*
Plugin Name
Copyright (C) <Year> <Developer> <Email Address>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <obs.h>
#include <obs-frontend-api.h>
#include <obs-module.h>
#include <plugin-support.h>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")

obs_scene_t *scene = 0;

bool flipAnything(obs_scene_t *s, obs_sceneitem_t *item, void *param)
{
	obs_sceneitem_set_rot(item, 180);
	obs_sceneitem_set_alignment(item, OBS_ALIGN_BOTTOM | OBS_ALIGN_RIGHT);
	UNUSED_PARAMETER(s);
	UNUSED_PARAMETER(param);

	return true;
}

void createAndStartOutput(bool bad)
{
	// Create a new output
	obs_output_t *output = obs_output_create(
		"rtmp_output", bad ? "bad_output" : "good_output", NULL, NULL);

	// Create a new service
	obs_data_t *serviceOptions = obs_data_create();
	obs_data_set_string(serviceOptions, "server", "rtmp://localhost/app");
	obs_data_set_string(serviceOptions, "key", bad ? "bad" : "good");
	obs_service_t *service = obs_service_create(
		"rtmp_custom", bad ? "bad_service" : "good_service",
		serviceOptions, NULL);
	if (!service) {
		obs_log(LOG_ERROR, "Failed to create service");
		obs_data_release(serviceOptions);
		return;
	}
	obs_output_set_service(output, service);

	// Create a new video encoder
	obs_data_t *videoOptions = obs_data_create();
	obs_data_set_int(videoOptions, "keyint_sec", 1);
	obs_data_set_string(videoOptions, "tune", "zerolatency");
	obs_encoder_t *videoEncoder = obs_video_encoder_create(
		"obs_x264", bad ? "bad_video_encoder" : "good_video_encoder",
		videoOptions, NULL);
	if (!videoEncoder) {
		obs_log(LOG_ERROR, "Failed to create video encoder");
		return;
	}
	obs_encoder_set_scaled_size(videoEncoder, 640, 360);

	///////////////////////////
	// FIXME: Set the video encoder to use the GPU scaling if this is bad output
	if (bad)
		obs_encoder_set_gpu_scale_type(videoEncoder,
					       OBS_SCALE_BILINEAR);
	///////////////////////////

	obs_view_t *view = obs_view_create();
	obs_view_set_source(view, 0, obs_scene_get_source(scene));

	// Get video from the view
	video_t *video = obs_view_add(view);

	// Set the video encoder to use the video from the view
	obs_encoder_set_video(videoEncoder, video);
	obs_output_set_video_encoder(output, videoEncoder);

	// Create a new audio encoder
	obs_encoder_t *audioEncoder = obs_audio_encoder_create(
		"ffmpeg_aac", bad ? "bad_audio_encoder" : "good_audio_encoder",
		NULL, 0, NULL);
	if (!audioEncoder) {
		obs_log(LOG_ERROR, "Failed to create audio encoder");
		return;
	}

	obs_encoder_set_audio(audioEncoder, obs_get_audio());
	obs_output_set_audio_encoder(output, audioEncoder, 0);

	// Start the output
	obs_output_start(output);
}

void obs_module_post_load(void)
{
	obs_log(LOG_INFO, "plugin post load");
}

void onFrontendEvent(enum obs_frontend_event event, void *private_data)
{
	UNUSED_PARAMETER(private_data);

	if (event == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
		// Duplicate a private scene from the current scene
		obs_source_t *currentSceneSource =
			obs_frontend_get_current_scene();
		scene = obs_scene_duplicate(
			obs_scene_from_source(currentSceneSource), "flip_scene",
			OBS_SCENE_DUP_PRIVATE_REFS);
		if (!scene) {
			obs_log(LOG_ERROR, "Failed to duplicate scene");
			return;
		}

		// make all scene items flip in this private scene
		obs_scene_enum_items(scene, flipAnything, NULL);

		// Start a good output
		createAndStartOutput(false);
		// Start a bad output
		createAndStartOutput(true);
	}
}

bool obs_module_load(void)
{
	obs_log(LOG_INFO, "plugin loaded successfully (version %s)",
		PLUGIN_VERSION);
	obs_frontend_add_event_callback(onFrontendEvent, NULL);
	return true;
}

void obs_module_unload(void)
{
	obs_log(LOG_INFO, "plugin unloaded");
}
