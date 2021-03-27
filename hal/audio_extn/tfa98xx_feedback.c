/*
 * Copyright (c) 2013 - 2016, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_TAG "audio_hw_tfa98xx_feedback"
/*#define LOG_NDEBUG 0*/
#define LOG_NDDEBUG 0

#include <errno.h>
#include <math.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <dirent.h>
#include "audio_hw.h"
#include "platform.h"
#include "platform_api.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <cutils/properties.h>
#include "audio_extn.h"


#ifdef TFA98XX_ENABLED

static struct pcm_config pcm_config_tfa98xx_fb = {
    .channels = 2,
    .rate = 48000,
    .period_size = 256,
    .period_count = 4,
    .format = PCM_FORMAT_S16_LE,
    .start_threshold = 0,
    .stop_threshold = INT_MAX,
    .avail_min = 0,
};

static struct pcm *pcm_tx;

static bool can_enable_feedback_on_device(snd_device_t snd_device)
{
    bool ret = false;

    if (snd_device == SND_DEVICE_OUT_SPEAKER ||
        snd_device == SND_DEVICE_OUT_SPEAKER_REVERSE ||
        snd_device == SND_DEVICE_OUT_SPEAKER_AND_ANC_HEADSET ||
        snd_device == SND_DEVICE_OUT_SPEAKER_AND_HDMI ||
        snd_device == SND_DEVICE_OUT_SPEAKER_AND_HEADPHONES ||
        snd_device == SND_DEVICE_OUT_SPEAKER_AND_LINE ||
        snd_device == SND_DEVICE_OUT_SPEAKER_AND_USB_HEADSET ||
        snd_device == SND_DEVICE_OUT_VOICE_SPEAKER ||
		snd_device == SND_DEVICE_OUT_VOICE_SPEAKER_2 ) {
        ret = true;
    }

    return ret;
}
int audio_extn_tfa98xx_start_feedback(struct audio_device *adev, 
													 snd_device_t snd_device)
{
    struct audio_usecase *uc_info_tx = NULL;
    int32_t pcm_dev_tx_id = -1, ret = 0;

	if (!can_enable_feedback_on_device(snd_device))
		return 0;

    ALOGV("%s: Entry", __func__);

    if (!adev) {
       ALOGE("%s: Invalid params", __func__);
       return -EINVAL;
    }

    if (!pcm_tx) {
		
		uc_info_tx = (struct audio_usecase *)calloc(1, sizeof(struct audio_usecase));
		if (!uc_info_tx) {
			return -ENOMEM;
		}
		
        uc_info_tx->id = USECASE_AUDIO_SPKR_CALIB_TX;
        uc_info_tx->type = PCM_CAPTURE;
        uc_info_tx->in_snd_device = SND_DEVICE_IN_CAPTURE_VI_FEEDBACK;
        uc_info_tx->out_snd_device = SND_DEVICE_NONE;
        list_init(&uc_info_tx->device_list);

        list_add_tail(&adev->usecase_list, &uc_info_tx->list);

        enable_snd_device(adev, SND_DEVICE_IN_CAPTURE_VI_FEEDBACK);
        enable_audio_route(adev, uc_info_tx);

        pcm_dev_tx_id = platform_get_pcm_device_id(uc_info_tx->id, PCM_CAPTURE);
        if (pcm_dev_tx_id < 0) {
            ALOGE("%s: Invalid pcm device for usecase (%d)",
                  __func__, uc_info_tx->id);
            ret = -ENODEV;
            goto exit;
        }
        pcm_tx = pcm_open(adev->snd_card,
                          pcm_dev_tx_id,
                          PCM_IN, &pcm_config_tfa98xx_fb);
        if (pcm_tx && !pcm_is_ready(pcm_tx)) {
            ALOGE("%s: %s", __func__, pcm_get_error(pcm_tx));
            ret = -EIO;
            goto exit;
        }
        if (pcm_start(pcm_tx) < 0) {
            ALOGE("%s: pcm start for TX failed", __func__);
            ret = -EINVAL;
        }
    }

exit:

     if (ret) {
        if (pcm_tx)
            pcm_close(pcm_tx);
		
        pcm_tx = NULL;

		disable_snd_device(adev, SND_DEVICE_IN_CAPTURE_VI_FEEDBACK);
		if (uc_info_tx) {
        	list_remove(&uc_info_tx->list);
        	uc_info_tx->id = USECASE_AUDIO_SPKR_CALIB_TX;
       		uc_info_tx->type = PCM_CAPTURE;
        	uc_info_tx->in_snd_device = SND_DEVICE_IN_CAPTURE_VI_FEEDBACK;
        	uc_info_tx->out_snd_device = SND_DEVICE_NONE;
       
        	disable_audio_route(adev, uc_info_tx);
        	free(uc_info_tx);
		}
    } 
	 
    ALOGV("%s: Exit", __func__);
    return ret;
}

void audio_extn_tfa98xx_stop_feedback(struct audio_device *adev, 
													 snd_device_t snd_device)
{
    struct audio_usecase *uc_info_tx;

	if (!can_enable_feedback_on_device(snd_device))
		return;

    ALOGV("%s: Entry", __func__);

    uc_info_tx = get_usecase_from_list(adev, USECASE_AUDIO_SPKR_CALIB_TX);
	
   	if (pcm_tx)
       	pcm_close(pcm_tx);
	
   	pcm_tx = NULL;
   	disable_snd_device(adev, SND_DEVICE_IN_CAPTURE_VI_FEEDBACK);
   	if (uc_info_tx) {
       	list_remove(&uc_info_tx->list);
       	disable_audio_route(adev, uc_info_tx);
       	free(uc_info_tx);
   	}
   
    ALOGV("%s: Exit", __func__);
}

#endif /*TFA98XX_ENABLED*/
