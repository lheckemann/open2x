/*
 *  - godori <ghcstop>, www.aesop-embedded.org
 *
 *    => Modified from Magiceyes code. October, 2004
 *
 *    => DIGNSYS Inc. < www.dignsys.com > developing from April 2005
 *
 */
#include "mmsp2_if.h"

extern int disp_mode;

int memfd 		= 0;
int mp4_fd 		= 0;
int vpp_fd 		= 0;
int	cx_fd 		= 0;
int init_flag 	= 0;
int resize_mode = 0;
static int frame_decode_count;
static int movie_width = 0;
static int movie_height = 0;
static int current_mode = 0;

static MP4D_INIT_PARAM 		mp4d_init;
static MP4D_INIT_BUF_PARAM 	mp4d_init_buf;
static MP4D_RUN_PARAM 		mp4d_run;
static MP4D_REL_PARAM 		mp4d_rel;

void *arm940_mem_start = NULL;

static unsigned char *rl_buf = NULL;
static unsigned short y_offset[6];
static unsigned short cb_offset[6];
static unsigned short cr_offset[6];

unsigned char  *dbuf;
MP4D_DISPLAY_PARAM mp4d_disp;

static FDC_INFO fdc_info;
static YUV_INFO yuv_info;
static SC_INFO 	sc_info;
static FDC_RUN_INFO fri;

extern int g_bTVMode;

#define MP4D_CODEC_INFO_SIZE 12
struct mp4d_codec_type_t CodecInfo[MP4D_CODEC_INFO_SIZE] =
{
    {mmioFOURCC('D', 'I', 'V', '3'), CODEC_VER_DIVX_311},
    {mmioFOURCC('d', 'i', 'v', '3'), CODEC_VER_DIVX_311},
    {mmioFOURCC('M', 'P', '4', '3'), CODEC_VER_DIVX_311},
    {mmioFOURCC('m', 'p', '4', '3'), CODEC_VER_DIVX_311},
    {mmioFOURCC('D', 'I', 'V', 'X'), CODEC_VER_DIVX_500},
    {mmioFOURCC('d', 'i', 'v', 'x'), CODEC_VER_DIVX_500},
    {mmioFOURCC('D', 'X', '5', '0'), CODEC_VER_DIVX_500},
    {mmioFOURCC('d', 'x', '5', '0'), CODEC_VER_DIVX_500},
    {mmioFOURCC('X', 'V', 'I', 'D'), CODEC_VER_XVID},
    {mmioFOURCC('x', 'v', 'i', 'd'), CODEC_VER_XVID},
    {mmioFOURCC('F', 'F', 'D', 'S'), CODEC_VER_DIVX_500},
    {mmioFOURCC('f', 'f', 'd', 's'), CODEC_VER_DIVX_500},
};

int mmsp2_940_interface_buffer_init(void)
{
    unsigned char  *addr;

    memfd = open("/dev/mem", O_RDWR | O_SYNC);
    if (memfd < 0)
    {
        printf("open /dev/mem failed\n");
        goto quit;
    }

    arm940_mem_start = mmap(0, 0x01000000, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, CODEC_BUFF_ADDR);
    if (arm940_mem_start == NULL)
    {
        printf("/dev/mem mmap() failed\n");
        goto quit;
    }

    dbuf = (unsigned char *) arm940_mem_start;
    rl_buf = (unsigned char *) arm940_mem_start + 0x280000;

    y_offset[0] = 0x0240;
    cb_offset[0] = 0x0244;
    cr_offset[0] = 0x0246;
    y_offset[1] = 0x0260;
    cb_offset[1] = 0x0264;
    cr_offset[1] = 0x0266;
    y_offset[2] = 0x0280;
    cb_offset[2] = 0x0284;
    cr_offset[2] = 0x0286;
    y_offset[3] = 0x02A0;
    cb_offset[3] = 0x02A4;
    cr_offset[3] = 0x02A6;
    y_offset[4] = 0x02C0;
    cb_offset[4] = 0x02C6;
    cr_offset[4] = 0x02C8;

    y_offset[5] = 0x0248;
    cb_offset[5] = 0x024C;
    cr_offset[5] = 0x024E;

    addr = (unsigned char *) arm940_mem_start + 0x00400000;
    memset(addr, 0x10, 0x200000);
    addr = (unsigned char *) arm940_mem_start;
    memset(addr, 0, 0x200000);

    return 1;

  quit:

    return -1;
}



int mmsp2_setup_940_interface_buffer(int width, int height, unsigned int vformat_fourcc)
{
    int             j;
    unsigned int    rlbuf_ptr;

    if ((width > MAX_WIDTH) || (height > MAX_HEIGHT))
    {
        printf("width and height are not supported\n");
        goto quit;
    }

    mp4d_init.Command 	= MP_CMD_INIT;
    mp4d_init.CodecType = MP_CODEC_MP4D;
    mp4d_init.Width 	= width;
    mp4d_init.Height 	= height;

    for (j = 0; j < MP4D_CODEC_INFO_SIZE; j++)
    {
        if (CodecInfo[j].format == vformat_fourcc)
        {
            printf("codec found = %d\n", j);
            break;
        }
    }

    if(j == 10)
    {
        printf("AVI compressor(fourcc) is not supported in current mmsp2 firmware!\n");
        goto quit;
    }

    mp4d_init.CodecVersion = CodecInfo[j].number;

#ifdef VIDEO_FW_041126
    if (mp4d_init.CodecVersion == CODEC_VER_DIVX_311)
        mp4d_init.CodecType = MP_CODEC_MP4D_DIVX3;

    mp4d_init.non_bframe_max = 4;
    mp4d_init.b_frame_max = 2;
#endif

    rlbuf_ptr = (unsigned long) rl_buf - (unsigned long) arm940_mem_start + CODEC_BUFF_ADDR;

    mp4d_init.RL_L = (unsigned short) (rlbuf_ptr & 0x0000FFFF);
    mp4d_init.RL_H = (unsigned short) (rlbuf_ptr >> 16);
    mp4d_init.RL_Length = 512;

    mp4d_init.mbp_a_addr_offset = 0x02F0;
    mp4d_init.mbp_b_addr_offset = 0x02E0;
    mp4d_init.qmat_addr_h = 0x0220;
    mp4d_init.qmat_addr_l = 0x0000;
    mp4d_init.pred_q_lb_offset = 0x0230;
    mp4d_init.arm940_offset = 0x0300;

	for(j=0; j < 10; j++) {
		if(ioctl(mp4_fd, IOCTL_DUALCPU_MP4D_INIT, &mp4d_init) == 1)
			break;

		printf("reopen dualcpu...\n");
		usleep(300000);

		close(mp4_fd);
		mp4_fd = open(PATH_DEV_DUALCPU, O_RDWR);
		if (mp4_fd < 0) {
			mp4_fd = open(PATH_DEV_DUALCPU_WITH_DEVFS, O_RDWR);
			if (mp4_fd < 0) {
				printf("IOCTL_DUALCPU_MP4D_INIT failed\n");
				goto quit;
			}
		}
	}

    if(read_mp4d_940_status(&mp4d_disp)<0)
    {
        printf("MP_CMD_INIT failed => goto quit\n");
        goto quit;
    }

    for (j = 0; j < 6; j++)
    {
        mp4d_init_buf.Command = MP_CMD_INIT_BUF;
        mp4d_init_buf.BufferType = ((j < 4) ? 1 : 0);
        mp4d_init_buf.BufferIdx = ((j < 4) ? j : (j - 4));
        mp4d_init_buf.Y_Offset = y_offset[j];
        mp4d_init_buf.CB_Offset = cb_offset[j];
        mp4d_init_buf.CR_Offset = cr_offset[j];

        if (ioctl(mp4_fd, IOCTL_DUALCPU_MP4D_INIT_BUF, &mp4d_init_buf) != 1)
        {
            printf("IOCTL_DUALCPU_MP4D_INIT_BUF failed\n");
            goto quit;
        }


        if (read_mp4d_940_status(&mp4d_disp) < 0)
        {
            printf("IOCTL_DUALCPU_MP4D_INIT_BUF timeout\n");
            goto quit;
        }
    }

    init_flag = 1;

    return 1;

quit:


    return -1;
}

int open_dualcpu_vpp(void)
{
	int retry = 1;

	while(retry) {
		mp4_fd = open(PATH_DEV_DUALCPU, O_RDWR);
		if (mp4_fd >= 0) break;

		mp4_fd = open(PATH_DEV_DUALCPU_WITH_DEVFS, O_RDWR);
		if (mp4_fd >= 0) break;

		if(retry) {
			printf("insert dualcpu module...\n");
			system("/sbin/modprobe dualcpu");
			retry = 0;
		} else {
			printf("open dualcpu driver failed\n");
			return -1;
		}
	}

    vpp_fd = open(PATH_DEV_VPP, O_RDWR);
    if (vpp_fd < 0)
    {
        vpp_fd = open(PATH_DEV_VPP_WITH_DEVFS, O_RDWR);
        if (vpp_fd < 0)
        {
            printf("open video post processor driver failed\n");
            return -1;
        }
    }

	if((ioctl(vpp_fd, IOCTL_MMSP2_DPC_VSYNC_INTERRUPT_ENABLE ,NULL))<0)
	{
		close(vpp_fd);
		return -1;
	}

    return 1;
}

void close_dualcpu_vpp_unmap(void)
{
    int             i;

    if (mp4_fd)
    {
        if (init_flag == 1)
        {
            mp4d_rel.Command = MP_CMD_RELEASE;
		#ifdef VIDEO_FW_041126
		    if (mp4d_init.CodecVersion == CODEC_VER_DIVX_311)
		        mp4d_rel.CodecType = MP_CODEC_MP4D_DIVX3;
		    else
		#endif
	     	   mp4d_rel.CodecType = MP_CODEC_MP4D;

            i = 1000;
            do
            {
                if (ioctl(mp4_fd, IOCTL_DUALCPU_MP4D_RELEASE, &mp4d_rel) == 1)
                    break;
                usleep(10000);
                i--;
            }
            while (i > 0);

            if(i == 0)
            {
                printf("IOCTL_DUALCPU_MP4D_RELEASE failed\n");
            }
            else
            {
                if (read_mp4d_940_status(&mp4d_disp) < 0)
                {
                    printf("IOCTL_DUALCPU_MP4D_RELEASE timeout\n");
                }
            }
        }
        close(mp4_fd);
    }

    if (vpp_fd)
    {
		ioctl(vpp_fd, IOCTL_MMSP2_STOP_SC, NULL);
        ioctl(vpp_fd, IOCTL_MMSP2_YUVA_OFF, NULL);
		ioctl(vpp_fd, IOCTL_MMSP2_RGB_OFF, 1);

		ioctl(vpp_fd, IOCTL_MMSP2_DPC_VSYNC_INTERRUPT_DISABLE ,NULL);

        close(vpp_fd);
    }

    if (arm940_mem_start)
        munmap(arm940_mem_start, 0x01000000);

    if (memfd)
        close(memfd);

}

inline int read_mp4d_940_status(MP4D_DISPLAY_PARAM * mp4d_940)
{

    if(read(mp4_fd, (void *)mp4d_940, sizeof(MP4D_DISPLAY_PARAM)) <= 0)
    {
        printf("dualcpu read error");
        return -1;
    }

    return 0;
}

int set_FDC_YUVB_plane(int width, int height)
{
	movie_width = width;
	movie_height = height;

   	fdc_info.VSync = 0;
    fdc_info.SrcField = 0;
    fdc_info.Chroma = FDC_CHROMA_420;
    fdc_info.Rotate = FDC_ROT_000;

    fdc_info.DstField = 0;

	fdc_info.ToScaler = 1;

	fdc_info.Width = width;
	fdc_info.Height = height;

    fdc_info.Y_Offset = y_offset[0];
    fdc_info.CB_Offset = cb_offset[0];
    fdc_info.CR_Offset = cr_offset[0];
    fdc_info.DstBaseAddr = 0;

	ioctl(vpp_fd, IOCTL_MMSP2_SET_FDC, &fdc_info);

	current_mode = -1;
	return change_resize_mode();
}

int change_resize_mode()
{
	unsigned int width=movie_width;
	unsigned int height=movie_height;
	unsigned int cal_width=width;
	unsigned int cal_height=height;
	unsigned int disp_width, disp_height;
	float ratio1=1.0, ratio2=1.0, cal_ratio=1.0;
	int skip=0;

	frame_decode_count = 0;
	if(disp_mode == DISPLAY_TV)resize_mode = 0;
	while(resize_mode < 0) resize_mode += 4;
	resize_mode %= 4;
	if(current_mode == resize_mode) return;
	current_mode = resize_mode;

	if ((disp_mode == DISPLAY_TV) && (g_bTVMode == 0)) {
		disp_width = TV_NTSC_ENC_WIDTH;
		disp_height = TV_NTSC_ENC_HEIGHT;
	} else if (disp_mode == DISPLAY_TV) {
		disp_width = TV_PAL_ENC_WIDTH;
		disp_height = TV_PAL_ENC_HEIGHT;
	} else {
		disp_width = LCD_WIDTH;
		disp_height = LCD_HEIGHT;
	}

	ratio1 = (float)width/(float)disp_width;
	ratio2 = (float)height/(float)disp_height;

	switch(current_mode) {
		case 0:
			cal_ratio = (ratio1 > ratio2) ? ratio1 : ratio2;
			break;
		case 1:
			cal_ratio = (ratio1 + ratio2) / 2;
			break;
		case 2:
			cal_ratio = ratio2;
			break;
		case 3:
			cal_ratio = 0;
			break;
	}

	if(cal_ratio > 0) {
		cal_width = ((unsigned short)(width/cal_ratio+0.5) + 1) & ~1;
		cal_height = (unsigned short)(height/cal_ratio+0.5);
	} else {
		cal_width = disp_width;
		cal_height = disp_height;
	}

    ioctl(vpp_fd, IOCTL_MMSP2_YUVA_OFF, NULL);
    ioctl(vpp_fd, IOCTL_MMSP2_YUVB_OFF, NULL);

	sc_info.Src = SC_FRM_FDC;
    sc_info.Dst = SC_TO_MEM;
    sc_info.NumOfFrame = SC_1_TIME;
    sc_info.WriteForm = SC_YCBYCR;
    sc_info.WriteField = SC_WR_FRAME;
    sc_info.hSrcMirror = 0;
    sc_info.vSrcMirror = 0;
    sc_info.hDstMirror = 0;
    sc_info.vDstMirror = 0;
    sc_info.SrcOddAddr = 0;
    sc_info.SrcEvenAddr = 0;
    sc_info.DstAddr = 0x03700000;
    sc_info.Y_Offset = 0;
    sc_info.CB_Offset = 0;
    sc_info.CR_Offset = 0;

	sc_info.SrcWidth	=	width;
	sc_info.SrcHeight	=	height;
	sc_info.SrcStride	=	width*2;
	sc_info.DstWidth	=	cal_width;
	sc_info.DstHeight	=	cal_height;
	sc_info.DstStride	=	cal_width*2;

	ioctl(vpp_fd, IOCTL_MMSP2_SET_SC, &sc_info);

 	yuv_info.Src = 0;
    yuv_info.Priority = MLC_YUV_PRIO_A;
    yuv_info.vMirrorTop = MLC_YUV_MIRROR_OFF;
    yuv_info.hMirrorTop = MLC_YUV_MIRROR_OFF;
    yuv_info.vMirrorBot = MLC_YUV_MIRROR_OFF;
    yuv_info.hMirrorBot = MLC_YUV_MIRROR_OFF;

	yuv_info.SrcWidthTop 	= (disp_width > cal_width) ? cal_width : disp_width;
	yuv_info.SrcHeightTop 	= cal_height;
	yuv_info.DstWidthTop 	= (disp_width > cal_width) ? cal_width : disp_width;
	yuv_info.DstHeightTop 	= cal_height;
	yuv_info.StrideTop 		= cal_width*2;

	if(disp_width < cal_width)
		skip = (cal_width - disp_width) & ~1;

	if(disp_width > cal_width) {
		yuv_info.StartX = (disp_width-cal_width)/2;
	} else {
		yuv_info.StartX = 0;
	}

	if(disp_mode == DISPLAY_TV) {
		yuv_info.StartX = 12;
		/*
		skip -= 24;
		if(skip < 0) {
			yuv_info.StartX -= (skip / 2);
			skip = 0;
		}
		*/
	}

	yuv_info.StartY = (disp_height-cal_height)/2;

	yuv_info.EndX = yuv_info.StartX;
    yuv_info.EndX += (disp_width > cal_width) ? cal_width : disp_width;
    yuv_info.EndY = yuv_info.StartY + cal_height-1;
    yuv_info.BottomY = yuv_info.EndY;
	yuv_info.TopAddr = 0x03800000 + skip;
	yuv_info.BotAddr = 0x03800000 + skip;

    ioctl(vpp_fd, IOCTL_MMSP2_SET_YUVA, &yuv_info);

	printf("mode = %d, ratio = %.3f, width = %d, height = %d, startx = %d, endx = %d, skip = %d\n",current_mode,cal_ratio,cal_width,cal_height,yuv_info.StartX,yuv_info.EndX,skip);

    return 1;
}

inline int run_940_decoder(void)
{
    unsigned long bufaddr;

    mp4d_run.Command = MP_CMD_RUN;
	#ifdef VIDEO_FW_041126
    if(mp4d_init.CodecVersion == CODEC_VER_DIVX_311)
        mp4d_run.CodecType = MP_CODEC_MP4D_DIVX3;
    else
	#endif
		mp4d_run.CodecType = MP_CODEC_MP4D;

    bufaddr = (unsigned long) dbuf - (unsigned long) arm940_mem_start + CODEC_BUFF_ADDR;
    mp4d_run.StreamBufferPaddrL = (unsigned short) (bufaddr & 0x0000FFFF);
    mp4d_run.StreamBufferPaddrH = (unsigned short) (bufaddr >> 16);

    if(ioctl(mp4_fd, IOCTL_DUALCPU_MP4D_RUN, &mp4d_run) != 1)
    {
        printf("IOCTL_DUALCPU_MP4D_RUN failed\n");
        return -1;
    }

    return 1;
}

int run_940_decoder_onlyrun(void)
{
    if (ioctl(mp4_fd, IOCTL_DUALCPU_MP4D_RUN, &mp4d_run) != 1)
    {
        printf("IOCTL_DUALCPU_MP4D_RUN failed\n");
        return -1;
    }

    return 1;
}

inline void run_FDC_and_display(unsigned short yoffset, unsigned short cboffset, unsigned short croffset)
{
    fri.Y_Offset  = yoffset;
    fri.CB_Offset = cboffset;
    fri.CR_Offset = croffset;

	while(ioctl(vpp_fd, IOCTL_MMSP2_SC_BUSY, NULL));
	if(resize_mode != current_mode)
		change_resize_mode();

	ioctl(vpp_fd, IOCTL_MMSP2_START_FDC, &fri);
	ioctl(vpp_fd, IOCTL_MMSP2_START_SC, SC_STARTMEM);

	if(frame_decode_count++ < 4)
	{
		if(frame_decode_count == 3)
		{
			while(ioctl(vpp_fd, IOCTL_MMSP2_SC_BUSY, NULL));
			ioctl(vpp_fd, IOCTL_MMSP2_YUVA_ON, NULL);
		}
	}
}
