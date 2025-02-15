/*
 * X11CairoGradient.h
 *
 *  Created on: 19 дек. 2016 г.
 *      Author: sadko
 */

#ifndef UI_X11_X11CAIROGRADIENT_H_
#define UI_X11_X11CAIROGRADIENT_H_

#ifndef UI_X11_WS_H_INCL_
    #error "This header should not be included directly"
#endif /* UI_X11_WS_H_INCL_ */

namespace lsp
{
    namespace ws
    {
        namespace x11
        {
            class X11CairoGradient: public IGradient
            {
                protected:
                    cairo_pattern_t *pCP;

                public:
                    X11CairoGradient();
                    virtual ~X11CairoGradient();

                public:
                    void apply(cairo_t *cr);

                public:
                    virtual void add_color(float offset, float r, float g, float b, float a);
            };

            class X11CairoLinearGradient: public X11CairoGradient
            {
                public:
                    inline X11CairoLinearGradient(float x0, float y0, float x1, float y1)
                    {
                        pCP = cairo_pattern_create_linear(x0, y0, x1, y1);
                    };

                    virtual ~X11CairoLinearGradient();
            };

            class X11CairoRadialGradient: public X11CairoGradient
            {
                public:
                    inline X11CairoRadialGradient(float cx0, float cy0, float r0, float cx1, float cy1, float r1)
                    {
                        pCP = cairo_pattern_create_radial(cx0, cy0, r0, cx1, cy1, r1);
                    };

                    virtual ~X11CairoRadialGradient();
            };
        }
    }
} /* namespace lsp */

#endif /* UI_X11_X11CAIROGRADIENT_H_ */
