#include "evas_common.h"
#include "evas_blend_private.h"

static void scale_rgba_in_to_out_clip_sample_internal(RGBA_Image *src, RGBA_Image *dst, RGBA_Draw_Context *dc, int src_region_x, int src_region_y, int src_region_w, int src_region_h, int dst_region_x, int dst_region_y, int dst_region_w, int dst_region_h);

EAPI void
evas_common_scale_rgba_in_to_out_clip_sample(RGBA_Image *src, RGBA_Image *dst,
                                             RGBA_Draw_Context *dc,
                                             int src_region_x, int src_region_y,
                                             int src_region_w, int src_region_h,
                                             int dst_region_x, int dst_region_y,
                                             int dst_region_w, int dst_region_h)
{
   evas_common_scale_rgba_in_to_out_clip_cb
     (src, dst, dc,
      src_region_x, src_region_y, src_region_w, src_region_h,
      dst_region_x, dst_region_y, dst_region_w, dst_region_h,
      scale_rgba_in_to_out_clip_sample_internal);
}

EAPI void
evas_common_scale_rgba_in_to_out_clip_sample_do(const Cutout_Rects *reuse,
                                                const Eina_Rectangle *clip,
                                                RGBA_Image *src, RGBA_Image *dst,
                                                RGBA_Draw_Context *dc,
                                                int src_region_x, int src_region_y,
                                                int src_region_w, int src_region_h,
                                                int dst_region_x, int dst_region_y,
                                                int dst_region_w, int dst_region_h)
{
   Eina_Rectangle area;
   Cutout_Rect *r;
   int i;

   if (!reuse)
     {
        evas_common_draw_context_clip_clip(dc, clip->x, clip->y, clip->w, clip->h);
        scale_rgba_in_to_out_clip_sample_internal(src, dst, dc,
                                                  src_region_x, src_region_y,
                                                  src_region_w, src_region_h,
                                                  dst_region_x, dst_region_y,
                                                  dst_region_w, dst_region_h);
        return;
     }

   for (i = 0; i < reuse->active; ++i)
     {
        r = reuse->rects + i;

        EINA_RECTANGLE_SET(&area, r->x, r->y, r->w, r->h);
        if (!eina_rectangle_intersection(&area, clip)) continue ;
        evas_common_draw_context_set_clip(dc, area.x, area.y, area.w, area.h);
        scale_rgba_in_to_out_clip_sample_internal(src, dst, dc,
                                                  src_region_x, src_region_y,
                                                  src_region_w, src_region_h,
                                                  dst_region_x, dst_region_y,
                                                  dst_region_w, dst_region_h);
     }
}

static void
scale_rgba_in_to_out_clip_sample_internal(RGBA_Image *src, RGBA_Image *dst,
					  RGBA_Draw_Context *dc,
					  int src_region_x, int src_region_y,
					  int src_region_w, int src_region_h,
					  int dst_region_x, int dst_region_y,
					  int dst_region_w, int dst_region_h)
{
   int      x, y;
   int     *lin_ptr;
   DATA32  *buf, *dptr;
   DATA32 **row_ptr;
   DATA32  *ptr, *dst_ptr, *src_data, *dst_data;
   int      dst_clip_x, dst_clip_y, dst_clip_w, dst_clip_h;
   int      src_w, src_h, dst_w, dst_h;
   RGBA_Gfx_Func func;

   if (!(RECTS_INTERSECT(dst_region_x, dst_region_y, dst_region_w, dst_region_h, 0, 0, dst->cache_entry.w, dst->cache_entry.h)))
     return;
   if (!(RECTS_INTERSECT(src_region_x, src_region_y, src_region_w, src_region_h, 0, 0, src->cache_entry.w, src->cache_entry.h)))
     return;

   src_w = src->cache_entry.w;
   src_h = src->cache_entry.h;
   dst_w = dst->cache_entry.w;
   dst_h = dst->cache_entry.h;

   src_data = src->image.data;
   dst_data = dst->image.data;

   if (dc->clip.use)
     {
	dst_clip_x = dc->clip.x;
	dst_clip_y = dc->clip.y;
	dst_clip_w = dc->clip.w;
	dst_clip_h = dc->clip.h;
	if (dst_clip_x < 0)
	  {
	     dst_clip_w += dst_clip_x;
	     dst_clip_x = 0;
	  }
	if (dst_clip_y < 0)
	  {
	     dst_clip_h += dst_clip_y;
	     dst_clip_y = 0;
	  }
	if ((dst_clip_x + dst_clip_w) > dst_w)
	  dst_clip_w = dst_w - dst_clip_x;
	if ((dst_clip_y + dst_clip_h) > dst_h)
	  dst_clip_h = dst_h - dst_clip_y;
     }
   else
     {
	dst_clip_x = 0;
	dst_clip_y = 0;
	dst_clip_w = dst_w;
	dst_clip_h = dst_h;
     }

   if (dst_clip_x < dst_region_x)
     {
	dst_clip_w += dst_clip_x - dst_region_x;
	dst_clip_x = dst_region_x;
     }
   if ((dst_clip_x + dst_clip_w) > (dst_region_x + dst_region_w))
     dst_clip_w = dst_region_x + dst_region_w - dst_clip_x;
   if (dst_clip_y < dst_region_y)
     {
	dst_clip_h += dst_clip_y - dst_region_y;
	dst_clip_y = dst_region_y;
     }
   if ((dst_clip_y + dst_clip_h) > (dst_region_y + dst_region_h))
     dst_clip_h = dst_region_y + dst_region_h - dst_clip_y;

   if ((src_region_w <= 0) || (src_region_h <= 0) ||
       (dst_region_w <= 0) || (dst_region_h <= 0) ||
       (dst_clip_w <= 0) || (dst_clip_h <= 0))
     return;

   /* sanitise x */
   if (src_region_x < 0)
     {
	dst_region_x -= (src_region_x * dst_region_w) / src_region_w;
	dst_region_w += (src_region_x * dst_region_w) / src_region_w;
	src_region_w += src_region_x;
	src_region_x = 0;
     }
   if (src_region_x >= src_w) return;
   if ((src_region_x + src_region_w) > src_w)
     {
	dst_region_w = (dst_region_w * (src_w - src_region_x)) / (src_region_w);
	src_region_w = src_w - src_region_x;
     }
   if (dst_region_w <= 0) return;
   if (src_region_w <= 0) return;
   if (dst_clip_x < 0)
     {
	dst_clip_w += dst_clip_x;
	dst_clip_x = 0;
     }
   if (dst_clip_w <= 0) return;
   if (dst_clip_x >= dst_w) return;
   if (dst_clip_x < dst_region_x)
     {
	dst_clip_w += (dst_clip_x - dst_region_x);
	dst_clip_x = dst_region_x;
     }
   if ((dst_clip_x + dst_clip_w) > dst_w)
     {
	dst_clip_w = dst_w - dst_clip_x;
     }
   if (dst_clip_w <= 0) return;

   /* sanitise y */
   if (src_region_y < 0)
     {
	dst_region_y -= (src_region_y * dst_region_h) / src_region_h;
	dst_region_h += (src_region_y * dst_region_h) / src_region_h;
	src_region_h += src_region_y;
	src_region_y = 0;
     }
   if (src_region_y >= src_h) return;
   if ((src_region_y + src_region_h) > src_h)
     {
	dst_region_h = (dst_region_h * (src_h - src_region_y)) / (src_region_h);
	src_region_h = src_h - src_region_y;
     }
   if (dst_region_h <= 0) return;
   if (src_region_h <= 0) return;
   if (dst_clip_y < 0)
     {
	dst_clip_h += dst_clip_y;
	dst_clip_y = 0;
     }
   if (dst_clip_h <= 0) return;
   if (dst_clip_y >= dst_h) return;
   if (dst_clip_y < dst_region_y)
     {
	dst_clip_h += (dst_clip_y - dst_region_y);
	dst_clip_y = dst_region_y;
     }
   if ((dst_clip_y + dst_clip_h) > dst_h)
     {
	dst_clip_h = dst_h - dst_clip_y;
     }
   if (dst_clip_h <= 0) return;

   /* allocate scale lookup tables */
   lin_ptr = alloca(dst_clip_w * sizeof(int));
   row_ptr = alloca(dst_clip_h * sizeof(DATA32 *));

   /* figure out dst jump */
   //dst_jump = dst_w - dst_clip_w;

   /* figure out dest start ptr */
   dst_ptr = dst_data + dst_clip_x + (dst_clip_y * dst_w);

   if (dc->mul.use)
     func = evas_common_gfx_func_composite_pixel_color_span_get(src, dc->mul.col, dst, dst_clip_w, dc->render_op);
   else
     func = evas_common_gfx_func_composite_pixel_span_get(src, dst, dst_clip_w, dc->render_op);

   if ((dst_region_w == src_region_w) && (dst_region_h == src_region_h))
     {
#ifdef HAVE_PIXMAN
# ifdef PIXMAN_IMAGE_SCALE_SAMPLE        
        if ((src->pixman.im) && (dst->pixman.im) && ((!dc->mul.use) ||
                ((dc->mul.use) && (dc->mul.col == 0xffffffff))) &&
            ((dc->render_op == _EVAS_RENDER_COPY) ||
                (dc->render_op == _EVAS_RENDER_BLEND)))
          {
             pixman_op_t op = PIXMAN_OP_SRC; // _EVAS_RENDER_COPY
             if (dc->render_op == _EVAS_RENDER_BLEND)
               op = PIXMAN_OP_OVER;
             
             pixman_image_composite(op,
                                    src->pixman.im, NULL,
                                    dst->pixman.im,
                                    (dst_clip_x - dst_region_x) + src_region_x,
                                    (dst_clip_y - dst_region_y) + src_region_y,
                                    0, 0,
                                    dst_clip_x, dst_clip_y,
                                    dst_clip_w, dst_clip_h);
          }
        else
# endif          
#endif
          {
             ptr = src_data + ((dst_clip_y - dst_region_y + src_region_y) * src_w) + (dst_clip_x - dst_region_x) + src_region_x;
             for (y = 0; y < dst_clip_h; y++)
               {
		 /* * blend here [clip_w *] ptr -> dst_ptr * */
		 func(ptr, NULL, dc->mul.col, dst_ptr, dst_clip_w);

		 ptr += src_w;
		 dst_ptr += dst_w;
               }
	  }
     }
   else
     {
        /* fill scale tables */
	for (x = 0; x < dst_clip_w; x++)
          lin_ptr[x] = (((x + dst_clip_x - dst_region_x) * src_region_w) / dst_region_w) + src_region_x;
	for (y = 0; y < dst_clip_h; y++)
          row_ptr[y] = src_data + (((((y + dst_clip_y - dst_region_y) * src_region_h) / dst_region_h)
                                    + src_region_y) * src_w);
	/* scale to dst */
	dptr = dst_ptr;
#ifdef DIRECT_SCALE
	if ((!src->cache_entry.flags.alpha) &&
            (!dst->cache_entry.flags.alpha) &&
            (!dc->mul.use))
	  {
	     for (y = 0; y < dst_clip_h; y++)
	       {

		 dst_ptr = dptr;
		 for (x = 0; x < dst_clip_w; x++)
		   {
		     ptr = row_ptr[y] + lin_ptr[x];
		     *dst_ptr = *ptr;
		     dst_ptr++;
		   }

		 dptr += dst_w;
               }
	  }
	else
#endif
	  {
	    /* a scanline buffer */
             buf = alloca(dst_clip_w * sizeof(DATA32));
             for (y = 0; y < dst_clip_h; y++)
               {
		 dst_ptr = buf;
		 for (x = 0; x < dst_clip_w; x++)
		   {
		     ptr = row_ptr[y] + lin_ptr[x];
		     *dst_ptr = *ptr;
		     dst_ptr++;
		   }
		 /* * blend here [clip_w *] buf -> dptr * */
		 func(buf, NULL, dc->mul.col, dptr, dst_clip_w);

		 dptr += dst_w;
               }
	  }
     }
}
