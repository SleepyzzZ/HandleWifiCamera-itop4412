/*
 * Copyright 2010 Samsung Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
//#include <utils/Log.h>

#include "SsbSipMfcApi.h"
#include "mfc_interface.h"

#define _MFCLIB_MAGIC_NUMBER     0x92241001

void LOGE(char *m)
{
    printf(m);
}

void LOGV(char *m)
{
    printf(m);
}

void *SsbSipMfcEncOpen(void)
{
    int hMFCOpen;
    _MFCLIB *pCTX;
    unsigned int mapped_addr;
    int mapped_size;
    struct mfc_common_args EncArg;

	if(access(SAMSUNG_MFC_DEV_NAME, F_OK) != 0) {
		LOGE("[SsbSipMfcEncOpen]: MFC device node not exists\n");
		return NULL;
	}
    hMFCOpen = open(SAMSUNG_MFC_DEV_NAME, O_RDWR | O_NDELAY);
    if (hMFCOpen < 0) {
        LOGE("[SsbSipMfcEncOpen]: MFC Open failure\n");
        return NULL;
    }

    pCTX = (_MFCLIB *)malloc(sizeof(_MFCLIB));
    if (pCTX == NULL) {
        LOGE("[SsbSipMfcEncOpen]: malloc failed.\n");
        close(hMFCOpen);
        return NULL;
    }
    memset(pCTX, 0, sizeof(_MFCLIB));

    mapped_size = ioctl(hMFCOpen, IOCTL_MFC_GET_MMAP_SIZE, &EncArg);
    if ((mapped_size <= 0)||(EncArg.ret_code != MFC_RET_OK)) {
        LOGE("[SsbSipMfcEncOpen]: IOCTL_MFC_GET_MMAP_SIZE failed");
        //mapped_size = MMAP_BUFFER_SIZE_MMAP;
        free(pCTX);
        close(hMFCOpen);
        return NULL;
    }
    fprintf(stdout, "[SsbSipMfcEncOpen] mapped_size = %d\n", mapped_size);

    mapped_addr = (unsigned int)mmap(0, mapped_size, PROT_READ | PROT_WRITE, MAP_SHARED, hMFCOpen, 0);
    if (!mapped_addr) {
        LOGE("[SsbSipMfcEncOpen]: FIMV5.0 driver address mapping failed\n");
        free(pCTX);
        close(hMFCOpen);
        return NULL;
    }

    pCTX->magic 			= _MFCLIB_MAGIC_NUMBER;
    pCTX->hMFC 				= hMFCOpen;
    pCTX->mapped_addr 		= mapped_addr;
    pCTX->mapped_size 		= mapped_size;
    pCTX->inter_buff_status = MFC_USE_NONE;

    return (void *)pCTX;
}

SSBSIP_MFC_ERROR_CODE SsbSipMfcEncInit(void *openHandle, void *param)
{
    int ret_code;
    //int dpbBufSize;

    _MFCLIB *pCTX;
    struct mfc_common_args EncArg;
    //mfc_common_args user_addr_arg, phys_addr_arg;
    SSBSIP_MFC_ENC_H264_PARAM *h264_arg;
    SSBSIP_MFC_ENC_MPEG4_PARAM *mpeg4_arg;
    SSBSIP_MFC_ENC_H263_PARAM *h263_arg;
    //SSBSIP_MFC_CODEC_TYPE codec_type;

    pCTX = (_MFCLIB *)openHandle;
    memset(&EncArg, 0, sizeof(struct mfc_common_args));

	pCTX->encode_cnt = 0;
	
	mpeg4_arg = (SSBSIP_MFC_ENC_MPEG4_PARAM *)param;
	if(mpeg4_arg->codecType == MPEG4_ENC) {
		pCTX->codecType = MPEG4_ENC;
	} else {
		h263_arg = (SSBSIP_MFC_ENC_H263_PARAM *)param;
		if (h263_arg->codecType == H263_ENC) {
			pCTX->codecType = H263_ENC;
		} else {
			h264_arg = (SSBSIP_MFC_ENC_H264_PARAM*)param;
			if(h264_arg->codecType == H264_ENC) {
				pCTX->codecType = H264_ENC;
			} else {
				LOGE("[SsbSipMfcEncInit]: Undefined codec type\n");
				return MFC_RET_INVALID_PARAM;
			}
		}
	}

    LOGV("SsbSipMfcEncInit: Encode Init start\n");
    switch (pCTX->codecType) {
    case MPEG4_ENC:
        LOGV("SsbSipMfcEncInit: MPEG4 Encode\n");
        mpeg4_arg = (SSBSIP_MFC_ENC_MPEG4_PARAM *)param;

        pCTX->width = mpeg4_arg->SourceWidth;
        pCTX->height = mpeg4_arg->SourceHeight;
        break;

    case H263_ENC:
        LOGV("SsbSipMfcEncInit: H263 Encode\n");
        h263_arg = (SSBSIP_MFC_ENC_H263_PARAM *)param;

        pCTX->width = h263_arg->SourceWidth;
        pCTX->height = h263_arg->SourceHeight;
        break;

    case H264_ENC:
        LOGV("[SsbSipMfcEncInit]: H264 Encode\n");
        h264_arg = (SSBSIP_MFC_ENC_H264_PARAM *)param;

        pCTX->width = h264_arg->SourceWidth;
        pCTX->height = h264_arg->SourceHeight;
        break;

    default:
        break;
    }

    switch (pCTX->codecType) {
    case MPEG4_ENC:
        /*mpeg4_arg = (SSBSIP_MFC_ENC_MPEG4_PARAM *)param;

        EncArg.args.enc_init_mpeg4.in_codec_type = pCTX->codec_type;
        EncArg.args.enc_init_mpeg4.in_profile_level = ENC_PROFILE_LEVEL(mpeg4_arg->ProfileIDC, mpeg4_arg->LevelIDC);

        EncArg.args.enc_init_mpeg4.in_width = mpeg4_arg->SourceWidth;
        EncArg.args.enc_init_mpeg4.in_height = mpeg4_arg->SourceHeight;
        EncArg.args.enc_init_mpeg4.in_gop_num = mpeg4_arg->IDRPeriod;
        if (mpeg4_arg->DisableQpelME)
            EncArg.args.enc_init_mpeg4.in_qpelME_enable = 0;
        else
            EncArg.args.enc_init_mpeg4.in_qpelME_enable = 1;

        EncArg.args.enc_init_mpeg4.in_MS_mode = mpeg4_arg->SliceMode;
        EncArg.args.enc_init_mpeg4.in_MS_size = mpeg4_arg->SliceArgument;

        if (mpeg4_arg->NumberBFrames > 2) {
            LOGE("SsbSipMfcEncInit: No such BframeNum is supported.\n");
            return MFC_RET_INVALID_PARAM;
        }
        EncArg.args.enc_init_mpeg4.in_BframeNum = mpeg4_arg->NumberBFrames;
        EncArg.args.enc_init_mpeg4.in_mb_refresh = mpeg4_arg->RandomIntraMBRefresh;*/

        /* rate control*/
        /*EncArg.args.enc_init_mpeg4.in_RC_frm_enable = mpeg4_arg->EnableFRMRateControl;
        if ((mpeg4_arg->QSCodeMin > 51) || (mpeg4_arg->QSCodeMax > 51)) {
            LOGE("SsbSipMfcEncInit: No such Min/Max QP is supported.\n");
            return MFC_RET_INVALID_PARAM;
        }
        EncArg.args.enc_init_mpeg4.in_RC_qbound = ENC_RC_QBOUND(mpeg4_arg->QSCodeMin, mpeg4_arg->QSCodeMax);
        EncArg.args.enc_init_mpeg4.in_RC_rpara = mpeg4_arg->CBRPeriodRf;*/

        /* pad control */
        /*EncArg.args.enc_init_mpeg4.in_pad_ctrl_on = mpeg4_arg->PadControlOn;
        if ((mpeg4_arg->LumaPadVal > 255) || (mpeg4_arg->CbPadVal > 255) || (mpeg4_arg->CrPadVal > 255)) {
            LOGE("SsbSipMfcEncInit: No such Pad value is supported.\n");
            return MFC_RET_INVALID_PARAM;
        }
        EncArg.args.enc_init_mpeg4.in_luma_pad_val = mpeg4_arg->LumaPadVal;
        EncArg.args.enc_init_mpeg4.in_cb_pad_val = mpeg4_arg->CbPadVal;
        EncArg.args.enc_init_mpeg4.in_cr_pad_val = mpeg4_arg->CrPadVal;

        EncArg.args.enc_init_mpeg4.in_time_increament_res = mpeg4_arg->TimeIncreamentRes;
        EncArg.args.enc_init_mpeg4.in_time_vop_time_increament = mpeg4_arg->VopTimeIncreament;
        EncArg.args.enc_init_mpeg4.in_RC_framerate = (mpeg4_arg->TimeIncreamentRes / mpeg4_arg->VopTimeIncreament);
        EncArg.args.enc_init_mpeg4.in_RC_bitrate = mpeg4_arg->Bitrate;
        if ((mpeg4_arg->FrameQp > 51) || (mpeg4_arg->FrameQp_P) > 51 || (mpeg4_arg->FrameQp_B > 51)) {
            LOGE("SsbSipMfcEncInit: No such FrameQp is supported.\n");
            return MFC_RET_INVALID_PARAM;
        }
        EncArg.args.enc_init_mpeg4.in_frame_qp = mpeg4_arg->FrameQp;
        if (mpeg4_arg->FrameQp_P)
            EncArg.args.enc_init_mpeg4.in_frame_P_qp = mpeg4_arg->FrameQp_P;
        else
            EncArg.args.enc_init_mpeg4.in_frame_P_qp = mpeg4_arg->FrameQp;
        if (mpeg4_arg->FrameQp_B)
            EncArg.args.enc_init_mpeg4.in_frame_B_qp = mpeg4_arg->FrameQp_B;
        else
            EncArg.args.enc_init_mpeg4.in_frame_B_qp = mpeg4_arg->FrameQp;*/

        break;

    case H263_ENC:
        /*h263_arg = (SSBSIP_MFC_ENC_H263_PARAM *)param;

        EncArg.args.enc_init_mpeg4.in_codec_type = pCTX->codec_type;
        EncArg.args.enc_init_mpeg4.in_profile_level = ENC_PROFILE_LEVEL(66, 40);
        EncArg.args.enc_init_mpeg4.in_width = h263_arg->SourceWidth;
        EncArg.args.enc_init_mpeg4.in_height = h263_arg->SourceHeight;
        EncArg.args.enc_init_mpeg4.in_gop_num = h263_arg->IDRPeriod;
        EncArg.args.enc_init_mpeg4.in_mb_refresh = h263_arg->RandomIntraMBRefresh;
        EncArg.args.enc_init_mpeg4.in_MS_mode = h263_arg->SliceMode;
        EncArg.args.enc_init_mpeg4.in_MS_size = 0;*/

        /* rate control*/
        /*EncArg.args.enc_init_mpeg4.in_RC_frm_enable = h263_arg->EnableFRMRateControl;
        if ((h263_arg->QSCodeMin > 51) || (h263_arg->QSCodeMax > 51)) {
            LOGE("SsbSipMfcEncInit: No such Min/Max QP is supported.\n");
            return MFC_RET_INVALID_PARAM;
        }
        EncArg.args.enc_init_mpeg4.in_RC_qbound = ENC_RC_QBOUND(h263_arg->QSCodeMin, h263_arg->QSCodeMax);
        EncArg.args.enc_init_mpeg4.in_RC_rpara = h263_arg->CBRPeriodRf;*/

        /* pad control */
        /*EncArg.args.enc_init_mpeg4.in_pad_ctrl_on = h263_arg->PadControlOn;
        if ((h263_arg->LumaPadVal > 255) || (h263_arg->CbPadVal > 255) || (h263_arg->CrPadVal > 255)) {
            LOGE("SsbSipMfcEncInit: No such Pad value is supported.\n");
            return MFC_RET_INVALID_PARAM;
        }
        EncArg.args.enc_init_mpeg4.in_luma_pad_val = h263_arg->LumaPadVal;
        EncArg.args.enc_init_mpeg4.in_cb_pad_val = h263_arg->CbPadVal;
        EncArg.args.enc_init_mpeg4.in_cr_pad_val = h263_arg->CrPadVal;

        EncArg.args.enc_init_mpeg4.in_RC_framerate = h263_arg->FrameRate;
        EncArg.args.enc_init_mpeg4.in_RC_bitrate = h263_arg->Bitrate;
        if (h263_arg->FrameQp > 51) {
            LOGE("SsbSipMfcEncInit: No such FrameQp is supported.\n");
            return MFC_RET_INVALID_PARAM;
        }
        EncArg.args.enc_init_mpeg4.in_frame_qp = h263_arg->FrameQp;
        if (h263_arg->FrameQp_P)
            EncArg.args.enc_init_mpeg4.in_frame_P_qp = h263_arg->FrameQp_P;
        else
            EncArg.args.enc_init_mpeg4.in_frame_P_qp = h263_arg->FrameQp;*/

        break;

    case H264_ENC:
        h264_arg = (SSBSIP_MFC_ENC_H264_PARAM *)param;

		EncArg.args.enc_init.cmn.in_codec_type = H264_ENC;

        EncArg.args.enc_init.cmn.in_width = h264_arg->SourceWidth;
        EncArg.args.enc_init.cmn.in_height = h264_arg->SourceHeight;
        EncArg.args.enc_init.cmn.in_gop_num = h264_arg->IDRPeriod;

		if ((h264_arg->SliceMode == 0)||(h264_arg->SliceMode == 1)||
            (h264_arg->SliceMode == 2)||(h264_arg->SliceMode == 4)) {
            EncArg.args.enc_init.cmn.in_ms_mode = h264_arg->SliceMode;
        } else {
            printf("[SsbSipMfcEncInit]: No such slice mode is supported\n");
            return MFC_RET_INVALID_PARAM;
        }
        EncArg.args.enc_init.cmn.in_ms_arg = h264_arg->SliceArgument;
        EncArg.args.enc_init.cmn.in_mb_refresh = h264_arg->RandomIntraMBRefresh;

		 /* pad control */
        EncArg.args.enc_init.cmn.in_pad_ctrl_on = h264_arg->PadControlOn;
        if ((h264_arg->LumaPadVal > 255) || (h264_arg->CbPadVal > 255) || (h264_arg->CrPadVal > 255)) {
            printf("[SsbSipMfcEncInit]: No such Pad value is supported\n");
            return MFC_RET_INVALID_PARAM;
        }
        EncArg.args.enc_init.cmn.in_y_pad_val = h264_arg->LumaPadVal;
        EncArg.args.enc_init.cmn.in_cb_pad_val = h264_arg->CbPadVal;
        EncArg.args.enc_init.cmn.in_cr_pad_val = h264_arg->CrPadVal;
		 /* Input stream Mode  NV12_Linear or NV12_Tile*/
        EncArg.args.enc_init.cmn.in_frame_map = h264_arg->FrameMap;

		/* rate control*/
        EncArg.args.enc_init.cmn.in_rc_fr_en = h264_arg->EnableFRMRateControl;
        EncArg.args.enc_init.cmn.in_rc_bitrate = h264_arg->Bitrate;
        if ((h264_arg->FrameQp > 51) || (h264_arg->FrameQp_P > 51)) {
            printf("[SsbSipMfcEncInit]: No such FrameQp is supported\n");
            return MFC_RET_INVALID_PARAM;
        }
        EncArg.args.enc_init.cmn.in_vop_quant = h264_arg->FrameQp;
        EncArg.args.enc_init.cmn.in_vop_quant_p = h264_arg->FrameQp_P;

		if ((h264_arg->QSCodeMin > 51) || (h264_arg->QSCodeMax > 51)) {
            printf("[SsbSipMfcEncInit]: No such Min/Max QP is supported\n");
            return MFC_RET_INVALID_PARAM;
        }
        EncArg.args.enc_init.cmn.in_rc_qbound_min = h264_arg->QSCodeMin;
        EncArg.args.enc_init.cmn.in_rc_qbound_max = h264_arg->QSCodeMax;
        EncArg.args.enc_init.cmn.in_rc_rpara = h264_arg->CBRPeriodRf;
        /* H.264 Only */
        EncArg.args.enc_init.codec.h264.in_profile = h264_arg->ProfileIDC;
        EncArg.args.enc_init.codec.h264.in_level = h264_arg->LevelIDC;
        
        if (h264_arg->FrameQp_B > 51) {
            printf("[SsbSipMfcEncInit]: No such FrameQp is supported\n");
            return MFC_RET_INVALID_PARAM;
        }
        EncArg.args.enc_init.codec.h264.in_vop_quant_b = h264_arg->FrameQp_B;
        
        if (h264_arg->NumberBFrames > 2) {
            printf("[SsbSipMfcEncInit]: No such BframeNum is supported\n");
            return MFC_RET_INVALID_PARAM;
        }
        EncArg.args.enc_init.codec.h264.in_bframenum = h264_arg->NumberBFrames;
		EncArg.args.enc_init.codec.h264.in_interlace_mode = h264_arg->PictureInterlace;
		
		if ((h264_arg->NumberRefForPframes > 2)||(h264_arg->NumberReferenceFrames >2)) {
            printf("[SsbSipMfcEncInit]: No such ref Num is supported\n");
            return MFC_RET_INVALID_PARAM;
        }
        EncArg.args.enc_init.codec.h264.in_reference_num = h264_arg->NumberReferenceFrames;
        EncArg.args.enc_init.codec.h264.in_ref_num_p = h264_arg->NumberRefForPframes;
        EncArg.args.enc_init.codec.h264.in_rc_framerate = h264_arg->FrameRate;
        EncArg.args.enc_init.codec.h264.in_rc_mb_en = h264_arg->EnableMBRateControl;

        EncArg.args.enc_init.codec.h264.in_rc_mb_dark_dis = h264_arg->DarkDisable;
        EncArg.args.enc_init.codec.h264.in_rc_mb_smooth_dis = h264_arg->SmoothDisable;
        EncArg.args.enc_init.codec.h264.in_rc_mb_static_dis = h264_arg->StaticDisable;
        EncArg.args.enc_init.codec.h264.in_rc_mb_activity_dis = h264_arg->ActivityDisable;
        
        EncArg.args.enc_init.codec.h264.in_deblock_dis = h264_arg->LoopFilterDisable;
        if ((abs(h264_arg->LoopFilterAlphaC0Offset) > 6) || (abs(h264_arg->LoopFilterBetaOffset) > 6)) {
            printf("[SsbSipMfcEncInit]: No such AlphaC0Offset or BetaOffset is supported\n");
            return MFC_RET_INVALID_PARAM;
        }
        EncArg.args.enc_init.codec.h264.in_deblock_alpha_c0 = h264_arg->LoopFilterAlphaC0Offset;
        EncArg.args.enc_init.codec.h264.in_deblock_beta = h264_arg->LoopFilterBetaOffset;
        
        EncArg.args.enc_init.codec.h264.in_symbolmode = h264_arg->SymbolMode;
        EncArg.args.enc_init.codec.h264.in_transform8x8_mode = h264_arg->Transform8x8Mode;
        
        /* FIXME: is it removed? */
        EncArg.args.enc_init.codec.h264.in_md_interweight_pps = 300;
        EncArg.args.enc_init.codec.h264.in_md_intraweight_pps = 170;
        
        /* default setting */
        /*EncArg.args.enc_init_h264.in_md_interweight_pps = 0;
        EncArg.args.enc_init_h264.in_md_intraweight_pps = 0;*/
        break;

    default:
    	LOGE("[SsbSipMfcEncInit]: No such codec type is supported\n");
        return MFC_RET_INVALID_PARAM;
    }
    EncArg.args.enc_init.cmn.in_mapped_addr = pCTX->mapped_addr;

    ret_code = ioctl(pCTX->hMFC, IOCTL_MFC_ENC_INIT, &EncArg);
    if (EncArg.ret_code != MFC_RET_OK) {
        LOGE("[SsbSipMfcEncInit]: IOCTL_MFC_ENC_INIT (%d) failed\n");
        return MFC_RET_ENC_INIT_FAIL;
    }
	
	pCTX->virStrmBuf = EncArg.args.enc_init.cmn.out_u_addr.strm_ref_y;
    pCTX->phyStrmBuf = EncArg.args.enc_init.cmn.out_p_addr.strm_ref_y;

	pCTX->sizeStrmBuf = MAX_ENCODER_OUTPUT_BUFFER_SIZE;
    pCTX->encodedHeaderSize = EncArg.args.enc_init.cmn.out_header_size;
    
    pCTX->virMvRefYC = EncArg.args.enc_init.cmn.out_u_addr.mv_ref_yc;
    pCTX->inter_buff_status |= MFC_USE_STRM_BUFF;

    return MFC_RET_OK;
}

SSBSIP_MFC_ERROR_CODE SsbSipMfcEncExe(void *openHandle)
{
    int ret_code;
    _MFCLIB *pCTX;
    struct mfc_common_args EncArg;

    if (openHandle == NULL) {
        LOGE("SsbSipMfcEncExe: openHandle is NULL\n");
        return MFC_RET_INVALID_PARAM;
    }

    pCTX = (_MFCLIB *)openHandle;

    memset(&EncArg, 0x00, sizeof(struct mfc_common_args));

    EncArg.args.enc_exe.in_codec_type = pCTX->codecType;
    EncArg.args.enc_exe.in_Y_addr = (unsigned int)pCTX->phyFrmBuf.luma;
    EncArg.args.enc_exe.in_CbCr_addr = (unsigned int)pCTX->phyFrmBuf.chroma;
#if 0 /* peter for debug */
    EncArg.args.enc_exe.in_Y_addr_vir    = (unsigned int)pCTX->virFrmBuf.luma;
    EncArg.args.enc_exe.in_CbCr_addr_vir = (unsigned int)pCTX->virFrmBuf.chroma;
#endif
	EncArg.args.enc_exe.in_frametag = pCTX->inframetag;
	if (pCTX->encode_cnt == 0) {
        EncArg.args.enc_exe.in_strm_st   = (unsigned int)pCTX->phyStrmBuf;
        EncArg.args.enc_exe.in_strm_end  = (unsigned int)pCTX->phyStrmBuf + pCTX->sizeStrmBuf;
    } else {
        EncArg.args.enc_exe.in_strm_st = (unsigned int)pCTX->phyStrmBuf + (MAX_ENCODER_OUTPUT_BUFFER_SIZE/2);
        EncArg.args.enc_exe.in_strm_end = (unsigned int)pCTX->phyStrmBuf + pCTX->sizeStrmBuf + (MAX_ENCODER_OUTPUT_BUFFER_SIZE/2);
    }
    
    ret_code = ioctl(pCTX->hMFC, IOCTL_MFC_ENC_EXE, &EncArg);
    if (EncArg.ret_code != MFC_OK) {
        printf("[SsbSipMfcEncExe]: IOCTL_MFC_ENC_EXE failed(ret : %d)", EncArg.ret_code);
        return MFC_RET_ENC_EXE_ERR;
    }
    
    pCTX->encodedDataSize = EncArg.args.enc_exe.out_encoded_size;
    pCTX->encodedframeType = EncArg.args.enc_exe.out_frame_type;
    pCTX->encodedphyFrmBuf.luma = EncArg.args.enc_exe.out_Y_addr;
    pCTX->encodedphyFrmBuf.chroma = EncArg.args.enc_exe.out_CbCr_addr;
    pCTX->outframetagtop = EncArg.args.enc_exe.out_frametag_top;
    pCTX->outframetagbottom = EncArg.args.enc_exe.out_frametag_bottom;
    
    return MFC_RET_OK;
}

SSBSIP_MFC_ERROR_CODE SsbSipMfcEncClose(void *openHandle)
{
    int ret_code;
    _MFCLIB *pCTX;
    struct mfc_common_args free_arg;

    if (openHandle == NULL) {
        LOGE("[SsbSipMfcEncClose]: openHandle is NULL\n");
        return MFC_RET_INVALID_PARAM;
    }

    pCTX = (_MFCLIB *)openHandle;

	/* FIXME: free buffer? */
    if (pCTX->inter_buff_status & MFC_USE_YUV_BUFF) {
        free_arg.args.mem_free.key = pCTX->virFrmBuf.luma;
        ret_code = ioctl(pCTX->hMFC, IOCTL_MFC_FREE_BUF, &free_arg);
    }

    if (pCTX->inter_buff_status & MFC_USE_STRM_BUFF) {
        free_arg.args.mem_free.key = pCTX->virStrmBuf;
        ret_code = ioctl(pCTX->hMFC, IOCTL_MFC_FREE_BUF, &free_arg);
        free_arg.args.mem_free.key = pCTX->virMvRefYC;
        ret_code = ioctl(pCTX->hMFC, IOCTL_MFC_FREE_BUF, &free_arg);
    }

    pCTX->inter_buff_status = MFC_USE_NONE;

    munmap((void *)pCTX->mapped_addr, pCTX->mapped_size);
    close(pCTX->hMFC);
    free(pCTX);

    return MFC_RET_OK;
}

SSBSIP_MFC_ERROR_CODE SsbSipMfcEncGetInBuf(void *openHandle, SSBSIP_MFC_ENC_INPUT_INFO *input_info)
{
    int ret_code;
    _MFCLIB *pCTX;
    struct mfc_common_args user_addr_arg, real_addr_arg;
    int y_size, c_size;
    int aligned_y_size, aligned_c_size;

    if (openHandle == NULL) {
        LOGE("[SsbSipMfcEncGetInBuf]: openHandle is NULL\n");
        return MFC_RET_INVALID_PARAM;
    }

    pCTX = (_MFCLIB *)openHandle;

	y_size = pCTX->width * pCTX->height;
	c_size = (pCTX->width * pCTX->height) >> 1;
	
	/* lenear: 2KB, tile: 8KB */
	aligned_y_size = Align(y_size, 64*BUF_L_UNIT);
	aligned_c_size = Align(c_size, 64*BUF_L_UNIT);
	
	/* Allocate luma & chroma buf */
	user_addr_arg.args.mem_alloc.type = ENCODER;
	user_addr_arg.args.mem_alloc.buff_size = aligned_y_size + aligned_c_size;
    user_addr_arg.args.mem_alloc.mapped_addr = pCTX->mapped_addr;
    
    ret_code = ioctl(pCTX->hMFC, IOCTL_MFC_GET_IN_BUF, &user_addr_arg);
    if (ret_code < 0) {
        printf("[SsbSipMfcEncGetInBuf]: IOCTL_MFC_GET_IN_BUF failed\n");
        return MFC_RET_ENC_GET_INBUF_FAIL;
    }
    
    pCTX->virFrmBuf.luma = pCTX->mapped_addr + user_addr_arg.args.mem_alloc.offset;
    pCTX->virFrmBuf.chroma = pCTX->mapped_addr + user_addr_arg.args.mem_alloc.offset + (unsigned int)aligned_y_size;
    
    real_addr_arg.args.real_addr.key = user_addr_arg.args.mem_alloc.offset;
    ret_code = ioctl(pCTX->hMFC, IOCTL_MFC_GET_REAL_ADDR, &real_addr_arg);
    if (ret_code  < 0) {
        printf("[SsbSipMfcEncGetInBuf]: IOCTL_MFC_GET_REAL_ADDR failed\n");
        return MFC_RET_ENC_GET_INBUF_FAIL;
    }
    pCTX->phyFrmBuf.luma = real_addr_arg.args.real_addr.addr;
    pCTX->phyFrmBuf.chroma = real_addr_arg.args.real_addr.addr + (unsigned int)aligned_y_size;
    
    pCTX->sizeFrmBuf.luma = (unsigned int)y_size;
    pCTX->sizeFrmBuf.chroma = (unsigned int)c_size;
    pCTX->inter_buff_status |= MFC_USE_YUV_BUFF;
    
    input_info->YPhyAddr = (void*)pCTX->phyFrmBuf.luma;
    input_info->CPhyAddr = (void*)pCTX->phyFrmBuf.chroma;
    input_info->YVirAddr = (void*)pCTX->virFrmBuf.luma;
    input_info->CVirAddr = (void*)pCTX->virFrmBuf.chroma;
    
    return MFC_RET_OK;
}

SSBSIP_MFC_ERROR_CODE SsbSipMfcEncSetInBuf(void *openHandle, SSBSIP_MFC_ENC_INPUT_INFO *input_info)
{
    _MFCLIB *pCTX;

    if (openHandle == NULL) {
        LOGE("SsbSipMfcEncSetInBuf: openHandle is NULL\n");
        return MFC_RET_INVALID_PARAM;
    }

    //LOGV("SsbSipMfcEncSetInBuf: input_info->YPhyAddr & input_info->CPhyAddr should be 64KB aligned\n");

    pCTX = (_MFCLIB *)openHandle;

    pCTX->phyFrmBuf.luma = (unsigned int)input_info->YPhyAddr;
    pCTX->phyFrmBuf.chroma = (unsigned int)input_info->CPhyAddr;
    pCTX->virFrmBuf.luma = (unsigned int)input_info->YVirAddr;
    pCTX->virFrmBuf.chroma = (unsigned int)input_info->CVirAddr;

    pCTX->sizeFrmBuf.luma = (unsigned int)input_info->YSize;
    pCTX->sizeFrmBuf.chroma = (unsigned int)input_info->CSize;

    return MFC_RET_OK;
}

SSBSIP_MFC_ERROR_CODE SsbSipMfcEncGetOutBuf(void *openHandle, SSBSIP_MFC_ENC_OUTPUT_INFO *output_info)
{
    _MFCLIB *pCTX;

    if (openHandle == NULL) {
        LOGE("[SsbSipMfcEncGetOutBuf]: openHandle is NULL\n");
        return MFC_RET_INVALID_PARAM;
    }

    pCTX = (_MFCLIB *)openHandle;

    output_info->headerSize = pCTX->encodedHeaderSize;
    output_info->dataSize = pCTX->encodedDataSize;
    output_info->frameType = pCTX->encodedframeType;
    /*output_info->StrmPhyAddr = (void *)pCTX->phyStrmBuf;
    output_info->StrmVirAddr = (void *)pCTX->virStrmBuf;*/
    
    if(pCTX->encode_cnt == 0) {
		output_info->StrmPhyAddr = (void *)pCTX->phyStrmBuf;
		output_info->StrmVirAddr = (unsigned char *)pCTX->virStrmBuf + pCTX->mapped_addr;
    } else {
    	output_info->StrmPhyAddr = (unsigned char *)pCTX->phyStrmBuf + (MAX_ENCODER_OUTPUT_BUFFER_SIZE/2);
    	output_info->StrmVirAddr = (unsigned char *)pCTX->virStrmBuf + pCTX->mapped_addr + (MAX_ENCODER_OUTPUT_BUFFER_SIZE/2);
    }

	pCTX->encode_cnt++;
	pCTX->encode_cnt %= 2;
	
	output_info->encodedYPhyAddr = (void*)pCTX->encodedphyFrmBuf.luma;
    output_info->encodedCPhyAddr = (void*)pCTX->encodedphyFrmBuf.chroma;

    return MFC_RET_OK;
}

/*SSBSIP_MFC_ERROR_CODE SsbSipMfcEncSetOutBuf(void *openHandle, void *phyOutbuf, void *virOutbuf, int outputBufferSize)
{
    _MFCLIB *pCTX;

    if (openHandle == NULL) {
        LOGE("SsbSipMfcEncSetOutBuf: openHandle is NULL\n");
        return MFC_RET_INVALID_PARAM;
    }

    pCTX = (_MFCLIB *)openHandle;

    pCTX->phyStrmBuf = (int)phyOutbuf;
    pCTX->virStrmBuf = (int)virOutbuf;
    pCTX->sizeStrmBuf = outputBufferSize;

    return MFC_RET_OK;
}*/

/*SSBSIP_MFC_ERROR_CODE SsbSipMfcEncSetConfig(void *openHandle, SSBSIP_MFC_ENC_CONF conf_type, void *value)
{
    int ret_code;
    _MFCLIB *pCTX;
    mfc_common_args EncArg;

    if (openHandle == NULL) {
        LOGE("SsbSipMfcEncSetConfig: openHandle is NULL\n");
        return MFC_RET_INVALID_PARAM;
    }

    if (value == NULL) {
        LOGE("SsbSipMfcEncSetConfig: value is NULL\n");
        return MFC_RET_INVALID_PARAM;
    }

    pCTX = (_MFCLIB *)openHandle;
    memset(&EncArg, 0x00, sizeof(mfc_common_args));

    switch (conf_type) {
    case MFC_ENC_SETCONF_FRAME_TYPE:
    case MFC_ENC_SETCONF_CHANGE_FRAME_RATE:
    case MFC_ENC_SETCONF_CHANGE_BIT_RATE:
    case MFC_ENC_SETCONF_ALLOW_FRAME_SKIP:
        EncArg.args.set_config.in_config_param = conf_type;
        EncArg.args.set_config.in_config_value[0] = *((unsigned int *) value);
        EncArg.args.set_config.in_config_value[1] = 0;
        break;

    case MFC_ENC_SETCONF_FRAME_TAG:
        pCTX->in_frametag = *((int *)value);
        return MFC_RET_OK;

    default:
        LOGE("SsbSipMfcEncSetConfig: No such conf_type is supported.\n");
        return MFC_RET_INVALID_PARAM;
    }

    ret_code = ioctl(pCTX->hMFC, IOCTL_MFC_SET_CONFIG, &EncArg);
    if (EncArg.ret_code != MFC_RET_OK) {
        //LOGE("SsbSipMfcEncSetConfig: IOCTL_MFC_SET_CONFIG failed(ret : %d)\n", EncArg.ret_code);
        LOGE("SsbSipMfcEncSetConfig: IOCTL_MFC_SET_CONFIG failed(ret : %d)\n");
        return MFC_RET_ENC_SET_CONF_FAIL;
    }

    return MFC_RET_OK;
}*/

/*SSBSIP_MFC_ERROR_CODE SsbSipMfcEncGetConfig(void *openHandle, SSBSIP_MFC_ENC_CONF conf_type, void *value)
{
    int ret_code;
    _MFCLIB *pCTX;
    mfc_common_args EncArg;

    pCTX = (_MFCLIB *)openHandle;

    if (openHandle == NULL) {
        LOGE("SsbSipMfcEncGetConfig: openHandle is NULL\n");
        return MFC_RET_INVALID_PARAM;
    }
    if (value == NULL) {
        LOGE("SsbSipMfcEncGetConfig: value is NULL\n");
        return MFC_RET_INVALID_PARAM;
    }

    pCTX = (_MFCLIB *)openHandle;
    memset(&EncArg, 0x00, sizeof(mfc_common_args));

    switch (conf_type) {
    case MFC_ENC_GETCONF_FRAME_TAG:
        *((unsigned int *)value) = pCTX->out_frametag_top;
        break;

    default:
        LOGE("SsbSipMfcEncGetConfig: No such conf_type is supported.\n");
        return MFC_RET_INVALID_PARAM;
    }

    return MFC_RET_OK;
}*/




/*void lineear_to_tile(unsigned char* p_linear_addr,unsigned char* p_tiled_addr,unsigned int x_size,unsigned int y_size){
    unsigned char *l=p_linear_addr;
    unsigned char *t=p_tiled_addr;
    unsigned int i,j;

    for(i=0;i<159;i++){
        for(j=0;j<255;j++){
            if(i<32){
                if(j<64){
                    *(t+j+i*64)=*(l+j+i*176);
                }else if(j<128){
                    *(t+j%64+i*64+2048)=*(l+j+i*176);
                }else if(j<176){
                    *(t+j%64+i*64+2048*6)=*(l+j+i*176);
                }
            }else if(i<64){
                if(j<64){
                    *(t+j+i%32*64+2048*2)=*(l+j+i*176);
                }else if(j<128){
                    *(t+j%64+i%32*64+2048*3)=*(l+j+i*176);
                }else if(j<176){
                    *(t+j%64+i%32*64+2048*4)=*(l+j+i*176);
                }
            }else if(i<96){
                if(j<64){
                    *(t+j+i%32*64+2048*8)=*(l+j+i*176);
                }else if(j<128){
                    *(t+j%64+i%32*64+2048*9)=*(l+j+i*176);
                }else if(j<176){
                    *(t+j%64+i%32*64+2048*14)=*(l+j+i*176);
                }
            }else if(i<128){
                if(j<64){
                    *(t+j+i%32*64+2048*10)=*(l+j+i*176);
                }else if(j<128){
                    *(t+j%64+i%32*64+2048*11)=*(l+j+i*176);
                }else if(j<176){
                    *(t+j%64+i%32*64+2048*12)=*(l+j+i*176);
                }
            }else if(i<144){
                if(j<64){
                    *(t+j+i%32*64+2048*16)=*(l+j+i*176);
                }else if(j<128){
                    *(t+j%64+i%32*64+2048*17)=*(l+j+i*176);
                }else if(j<176){
                    *(t+j%64+i%32*64+2048*18)=*(l+j+i*176);
                }
            }
        }
    }
}*/

/*void lineear_to_tile_c(unsigned char* p_linear_addr,unsigned char* p_tiled_addr,unsigned int x_size,unsigned int y_size)
{
    unsigned char *l=p_linear_addr;
    unsigned char *t=p_tiled_addr;
    unsigned int i,j;

    for(i=0;i<96;i++){
        for(j=0;j<255;j++){
            if(i<32){
                if(j<64){
                    *(t+j+i*64)=*(l+j+i*176);
                }else if(j<128){
                    *(t+j%64+i*64+2048)=*(l+j+i*176);
                }else if(j<176){
                    *(t+j%64+i*64+2048*6)=*(l+j+i*176);
                }
            }else if(i<64){
                if(j<64){
                    *(t+j+i%32*64+2048*2)=*(l+j+i*176);
                }else if(j<128){
                    *(t+j%64+i%32*64+2048*3)=*(l+j+i*176);
                }else if(j<176){
                    *(t+j%64+i%32*64+2048*4)=*(l+j+i*176);
                }
            }else if(i<72){
                if(j<64){
                    *(t+j+i%32*64+2048*8)=*(l+j+i*176);
                }else if(j<128){
                    *(t+j%64+i%32*64+2048*9)=*(l+j+i*176);
                }else if(j<176){
                    *(t+j%64+i%32*64+2048*10)=*(l+j+i*176);
                }
            }
        }
    }
}*/
