#include "config.h"
#include <stdio.h>
#include <stdlib.h>

#include "mp_msg.h"
#include "libmpdemux/mpeg_hdr.h"

#include "vd_internal.h"

static vd_info_t info = 
{
	"MPEG 1/2 Video passthrough",
	"mpegpes",
	"A'rpi",
	"A'rpi",
	"for hw decoders"
};

LIBVD_EXTERN(mpegpes)

//#include "libmpdemux/parse_es.h"

#include "libvo/video_out.h"

// to set/get/query special features/parameters
static int control(sh_video_t *sh,int cmd,void* arg,...){
    return CONTROL_UNKNOWN;
}

// init driver
static int init(sh_video_t *sh){
    return mpcodecs_config_vo(sh,sh->disp_w,sh->disp_h,IMGFMT_MPEGPES);
}

// uninit driver
static void uninit(sh_video_t *sh){
}

// decode a frame
static mp_image_t* decode(sh_video_t *sh,void* data,int len,int flags){
    mp_image_t* mpi;
    static vo_mpegpes_t packet;
    mp_mpeg_header_t picture;
    const unsigned char *d = data;

    if(len>10 && !d[0] && !d[1] && d[2]==1 && d[3]==0xB3) {
        float old_aspect = sh->aspect;
        int oldw = sh->disp_w, oldh = sh->disp_h;
        mp_header_process_sequence_header(&picture, &d[4]);
        sh->aspect = mpeg12_aspect_info(&picture);
        sh->disp_w = picture.display_picture_width;
        sh->disp_h = picture.display_picture_height;
        if(sh->aspect != old_aspect || sh->disp_w != oldw || sh->disp_h != oldh) {
            if(!mpcodecs_config_vo(sh, sh->disp_w,sh->disp_h,IMGFMT_MPEGPES))
                return 0;
        }
    }
    
    mpi=mpcodecs_get_image(sh, MP_IMGTYPE_EXPORT, 0, sh->disp_w, sh->disp_h);
    packet.data=data;
    packet.size=len;
    packet.timestamp=sh->timer*90000.0;
    packet.id=0x1E0; //+sh_video->ds->id;
    mpi->planes[0]=(uint8_t*)(&packet);
    return mpi;
}
