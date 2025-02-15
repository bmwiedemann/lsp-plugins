/*
 * CtlComboBox.cpp
 *
 *  Created on: 21 авг. 2017 г.
 *      Author: sadko
 */

#include <ui/ctl/ctl.h>

namespace lsp
{
    namespace ctl
    {
        CtlComboBox::CtlComboBox(CtlRegistry *src, LSPComboBox *widget): CtlWidget(src, widget)
        {
            pPort       = NULL;
            fMin        = 0.0f;
            fMax        = 0.0f;
            fStep       = 0.0f;
            idChange    = -1;
        }

        CtlComboBox::~CtlComboBox()
        {
            do_destroy();
        }

        void CtlComboBox::init()
        {
            CtlWidget::init();

            if (pWidget == NULL)
                return;

            LSPComboBox *cbox = static_cast<LSPComboBox *>(pWidget);

            // Initialize color controllers
            sColor.init_hsl(pRegistry, cbox, cbox->color(), A_COLOR, A_HUE_ID, A_SAT_ID, A_LIGHT_ID);
            sBgColor.init_basic(pRegistry, cbox, cbox->bg_color(), A_BG_COLOR);

            // Bind slots
            idChange = cbox->slots()->bind(LSPSLOT_CHANGE, slot_change, this);
        }

        void CtlComboBox::do_destroy()
        {
            if (pWidget == NULL)
                return;

            LSPComboBox *cbox = static_cast<LSPComboBox *>(pWidget);

            if (idChange >= 0)
            {
                cbox->slots()->unbind(LSPSLOT_CHANGE, idChange);
                idChange = -1;
            }
        }

        void CtlComboBox::destroy()
        {
            CtlWidget::destroy();
            do_destroy();
        }

        status_t CtlComboBox::slot_change(LSPWidget *sender, void *ptr, void *data)
        {
            CtlComboBox *_this    = static_cast<CtlComboBox *>(ptr);
            if (_this != NULL)
                _this->submit_value();
            return STATUS_OK;
        }

        void CtlComboBox::submit_value()
        {
            LSPComboBox *cbox = widget_cast<LSPComboBox>(pWidget);
            if (cbox == NULL)
            {
                lsp_trace("CBOX IS NULL");
                return;
            }
            lsp_trace("CBOX IS NOT NULL");
            ssize_t index = cbox->selected();

            float value = fMin + fStep * index;
            lsp_trace("index = %d, value=%f", int(index), value);

            pPort->set_value(value);
            pPort->notify_all();
        }

        void CtlComboBox::set(widget_attribute_t att, const char *value)
        {
            LSPComboBox *cbox = (pWidget != NULL) ? static_cast<LSPComboBox *>(pWidget) : NULL;

            switch (att)
            {
                case A_ID:
                    BIND_PORT(pRegistry, pPort, value);
                    break;
                case A_WIDTH:
                    if (cbox != NULL)
                        PARSE_INT(value, cbox->set_min_width(__));
                    break;
                case A_HEIGHT:
                    if (cbox != NULL)
                        PARSE_INT(value, cbox->set_min_height(__));
                    break;
                default:
                {
                    bool set = sBgColor.set(att, value);
                    set |= sColor.set(att, value);
                    if (!set)
                        CtlWidget::set(att, value);
                    break;
                }
            }
        }

        void CtlComboBox::notify(CtlPort *port)
        {
            CtlWidget::notify(port);

            if ((pPort == port) && (pWidget != NULL))
            {
                ssize_t index = (pPort->get_value() - fMin) / fStep;

                LSPComboBox *cbox = static_cast<LSPComboBox *>(pWidget);
                cbox->set_selected(index);
            }
        }

        void CtlComboBox::end()
        {
            if (pWidget != NULL)
            {
                LSPComboBox *cbox = static_cast<LSPComboBox *>(pWidget);
                const port_t *p = (pPort != NULL) ? pPort->metadata() : NULL;

                if (p != NULL)
                {
                    get_port_parameters(pPort->metadata(), &fMin, &fMax, &fStep);

                    if (p->unit == U_ENUM)
                    {
                        size_t value    = pPort->get_value();
                        size_t i        = 0;
                        LSPItemList *lst= cbox->items();

                        for (const char **item = p->items; (item != NULL) && (*item != NULL); ++item, ++i)
                        {
                            size_t key      = fMin + fStep * i;
                            lst->add(*item, key);
                            if (key == value)
                                cbox->set_selected(i);
                        }
                    }
                }
            }

            CtlWidget::end();
        }
    } /* namespace ctl */
} /* namespace lsp */
