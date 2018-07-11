/*
 * ext.h
 *
 *  Created on: 23 окт. 2015 г.
 *      Author: sadko
 */

#ifndef CORE_CONTAINER_LV2EXT_H_
#define CORE_CONTAINER_LV2EXT_H_

// LV2 includes
#include <lv2.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/atom/forge.h>
#include <lv2/lv2plug.in/ns/ext/atom/util.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>
#include <lv2/lv2plug.in/ns/ext/port-groups/port-groups.h>
#include <lv2/lv2plug.in/ns/ext/resize-port/resize-port.h>
#include <lv2/lv2plug.in/ns/ext/state/state.h>
#include <lv2/lv2plug.in/ns/ext/patch/patch.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include <lv2/lv2plug.in/ns/ext/port-props/port-props.h>
#include <lv2/lv2plug.in/ns/ext/worker/worker.h>
#include <lv2/lv2plug.in/ns/extensions/units/units.h>
#include <lv2/lv2plug.in/ns/extensions/ui/ui.h>

// Non-official features
#include <3rdparty/ardour/inline-display.h>

// Include common definitions
#include <container/const.h>

#ifndef LV2_ATOM__Object
    #define LV2_ATOM__Object        LV2_ATOM_PREFIX "Object"
#endif /* LV2_ATOM__Object */

namespace lsp
{
    #define LSP_LV2_ATOM_KEY_SIZE       (sizeof(uint32_t) * 2)
    #define LSP_LV2_SIZE_PAD(size)      ALIGN_SIZE((size + 0x200), 0x200)

    struct LV2Extensions;

    class LV2Serializable
    {
        protected:
            LV2Extensions          *pExt;
            LV2_URID                urid;

        public:
            explicit LV2Serializable(LV2Extensions *ext)
            {
                pExt        = ext;
                urid        = 0;
            }

            virtual ~LV2Serializable()
            {
            }

        public:
            virtual void serialize() {};

            virtual void deserialize(const void *data) {};

        public:
            inline LV2_URID         get_urid() const        { return urid; };
    };

    struct LV2Extensions
    {
        public:
            LV2_Atom_Forge          forge;

            // Extension interfaces
            LV2_URID_Map           *map;
            LV2_URID_Unmap         *unmap;
            LV2_Worker_Schedule    *sched;
            LV2_Inline_Display     *iDisplay;

            // State interface
            LV2_State_Store_Function    hStore;
            LV2_State_Retrieve_Function hRetrieve;
            LV2_State_Handle            hHandle;

            // Plugin URI and URID
            const char             *uriPlugin;
            LV2_URID                uridPlugin;

            // Miscellaneous URIDs
            LV2_URID                uridAtomTransfer;
            LV2_URID                uridEventTransfer;
            LV2_URID                uridObject;
            LV2_URID                uridState;
            LV2_URID                uridStateChange;
            LV2_URID                uridStateRequest;
            LV2_URID                uridConnectUI;
            LV2_URID                uridUINotification;
            LV2_URID                uridDisconnectUI;
            LV2_URID                uridMeshType;
            LV2_URID                uridPathType;
            LV2_URID                uridMidiEventType;
            LV2_URID                uridPatchGet;
            LV2_URID                uridPatchSet;
            LV2_URID                uridPatchMessage;
            LV2_URID                uridPatchResponse;
            LV2_URID                uridPatchProperty;
            LV2_URID                uridPatchValue;
            LV2_URID                uridAtomUrid;
            LV2_URID                uridChunk;

            LV2UI_Controller        ctl;
            LV2UI_Write_Function    wf;
            ssize_t                 nAtomIn;    // Atom input port identifier
            ssize_t                 nAtomOut;   // Atom output port identifier
            uint8_t                *pBuffer;    // Atom serialization buffer
            size_t                  nBufSize;   // Atom serialization buffer size

        public:
            inline LV2Extensions(const LV2_Feature* const* feat, const char *uri, LV2UI_Controller lv2_ctl, LV2UI_Write_Function lv2_write)
            {
                map                 = NULL;
                unmap               = NULL;
                sched               = NULL;
                iDisplay            = NULL;

                // Scan features
                if (feat != NULL)
                {
                    for (size_t i=0; feat[i]; ++i)
                    {
                        const LV2_Feature *f = feat[i];

                        if (!strcmp(f->URI, LV2_URID__map))
                            map = reinterpret_cast<LV2_URID_Map *>(f->data);
                        else if (!strcmp(f->URI, LV2_URID__unmap))
                            unmap = reinterpret_cast<LV2_URID_Unmap *>(f->data);
                        else if (!strcmp(f->URI, LV2_WORKER__schedule))
                            sched = reinterpret_cast<LV2_Worker_Schedule *>(f->data);
                        else if (!strcmp(f->URI, LV2_INLINEDISPLAY__queue_draw))
                            iDisplay = reinterpret_cast<LV2_Inline_Display *>(f->data);
                    }
                }

                ctl                 = lv2_ctl;
                wf                  = lv2_write;
                nAtomIn             = -1;
                nAtomOut            = -1;
                pBuffer             = NULL;
                nBufSize            = 0;

                uriPlugin           = uri;
                uridPlugin          = (map != NULL) ? map->map(map->handle, uriPlugin) : -1;

                if (map != NULL)
                    lv2_atom_forge_init(&forge, map);

                uridAtomTransfer    = map_uri(LV2_ATOM__atomTransfer);
                uridEventTransfer   = map_uri(LV2_ATOM__eventTransfer);
                uridObject          = map_uri(LV2_ATOM__Object);
                uridState           = map_primitive("state");
                uridConnectUI       = map_primitive("ui_connect");
                uridUINotification  = map_type("UINotification");
                uridDisconnectUI    = map_primitive("ui_disconnect");
                uridStateRequest    = map_type("StateRequest");
                uridStateChange     = map_type("StateChange");
                uridMeshType        = map_type("Mesh");
                uridPathType        = map_uri(LV2_ATOM__Path);
                uridMidiEventType   = map_uri(LV2_MIDI__MidiEvent);
                uridPatchGet        = map_uri(LV2_PATCH__Get);
                uridPatchSet        = map_uri(LV2_PATCH__Set);
                uridPatchMessage    = map_uri(LV2_PATCH__Message);
                uridPatchResponse   = map_uri(LV2_PATCH__Response);
                uridPatchProperty   = map_uri(LV2_PATCH__property);
                uridPatchValue      = map_uri(LV2_PATCH__value);
                uridAtomUrid        = map_uri(LV2_ATOM__URID);
                uridChunk           = map_uri(LV2_ATOM__Chunk);
            }

            ~LV2Extensions()
            {
                lsp_trace("destroy");

                // Drop atom buffer
                if (pBuffer != NULL)
                {
                    delete [] pBuffer;
                    pBuffer     = NULL;
                }
            }

        public:
            inline bool atom_supported() const
            {
                return map != NULL;
            }

            inline const char *unmap_urid(LV2_URID urid)
            {
                return (unmap != NULL) ? unmap->unmap(unmap->handle, urid) : NULL;
            }

            inline void write_data(
                    uint32_t         port_index,
                    uint32_t         buffer_size,
                    uint32_t         port_protocol,
                    const void*      buffer)
            {
                if ((ctl == NULL) || (wf == NULL))
                    return;
                wf(ctl, port_index, buffer_size, port_protocol, buffer);
            }

            inline LV2_Atom *forge_object(LV2_Atom_Forge_Frame* frame, LV2_URID id, LV2_URID otype)
            {
                const LV2_Atom_Object a = {
                    { sizeof(LV2_Atom_Object_Body), uridObject },
                    { id, otype }
                };
                return reinterpret_cast<LV2_Atom *>(
                    lv2_atom_forge_push(&forge, frame, lv2_atom_forge_write(&forge, &a, sizeof(a)))
                );
            }

            inline LV2_Atom_Vector *forge_vector(
                    uint32_t        child_size,
                    uint32_t        child_type,
                    uint32_t        n_elems,
                    const void*     elems)
            {
                return reinterpret_cast<LV2_Atom_Vector *>(
                    lv2_atom_forge_vector(&forge, child_size, child_type, n_elems, elems)
                );
            }

            inline LV2_Atom_Forge_Ref forge_sequence_head(LV2_Atom_Forge_Frame* frame, uint32_t unit)
            {
                return lv2_atom_forge_sequence_head(&forge, frame, unit);
            }

            inline LV2_Atom_Forge_Ref forge_key(LV2_URID key)
            {
                const uint32_t body[] = { key, 0 };
                return lv2_atom_forge_write(&forge, body, sizeof(body));
            }

            inline LV2_Atom_Forge_Ref forge_urid(LV2_URID urid)
            {
                const LV2_Atom_URID a = { { sizeof(LV2_URID), forge.URID }, urid };
                return lv2_atom_forge_write(&forge, &a, sizeof(LV2_Atom_URID));
            }

            inline LV2_Atom_Forge_Ref forge_path(const char *str)
            {
                size_t len = strlen(str);
                return lv2_atom_forge_typed_string(&forge, forge.Path, str, len);
            }

            inline void forge_pop(LV2_Atom_Forge_Frame* frame)
            {
                lv2_atom_forge_pop(&forge, frame);
            }

            inline LV2_Atom_Forge_Ref forge_float(float val)
            {
                const LV2_Atom_Float a = { { sizeof(float), forge.Float }, val };
                return lv2_atom_forge_primitive(&forge, &a.atom);
            }

            inline LV2_Atom_Forge_Ref forge_int(int32_t val)
            {
                const LV2_Atom_Int a    = { { sizeof(int32_t), forge.Int }, val };
                return lv2_atom_forge_primitive(&forge, &a.atom);
            }

            inline void forge_pad(size_t size)
            {
                lv2_atom_forge_pad(&forge, size);
            }

            inline LV2_Atom_Forge_Ref forge_raw(const void* data, size_t size)
            {
                return lv2_atom_forge_raw(&forge, data, size);
            }

            inline LV2_Atom_Forge_Ref forge_primitive(const LV2_Atom *atom)
            {
                return lv2_atom_forge_primitive(&forge, atom);
            }

            inline void forge_set_buffer(void* buf, size_t size)
            {
                lv2_atom_forge_set_buffer(&forge, reinterpret_cast<uint8_t *>(buf), size);
            }

            inline LV2_Atom_Forge_Ref forge_frame_time(int64_t frames)
            {
                return lv2_atom_forge_write(&forge, &frames, sizeof(frames));
            }

            inline LV2_URID map_uri(const char *fmt...)
            {
                if (map == NULL)
                    return -1;

                va_list vl;
                char tmpbuf[1024];

                va_start(vl, fmt);
                vsnprintf(tmpbuf, sizeof(tmpbuf), fmt, vl);
                va_end(vl);

                LV2_URID res = map->map(map->handle, tmpbuf);
                lsp_trace("URID for <%s> is %d", tmpbuf, int(res));
                return res;
            }

            inline LV2_URID map_port(const char *id)
            {
                return map_uri("%s/ports#%s", uriPlugin, id);
            }

            inline LV2_URID map_type(const char *id)
            {
                return map_uri("%s/types#%s", LSP_TYPE_URI(lv2), id);
            }

            inline LV2_URID map_primitive(const char *id)
            {
                return map_uri("%s/%s", uriPlugin, id);
            }

            inline void init_state_context(
                LV2_State_Store_Function    store,
                LV2_State_Retrieve_Function retrieve,
                LV2_State_Handle            handle,
                uint32_t                    flags,
                const LV2_Feature *const *  features
            )
            {
                hStore          = store;
                hRetrieve       = retrieve;
                hHandle         = handle;
            }

            inline void store_value(LV2_URID urid, LV2_URID type, const void *data, size_t size)
            {
                if ((hStore == NULL) || (hHandle == NULL))
                    return;
                hStore(hHandle, urid, data, size, type, LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);
            }

            inline const void *restore_value(LV2_URID urid, LV2_URID type, size_t *size)
            {
                if ((hRetrieve == NULL) || (hHandle == NULL))
                    return NULL;

                size_t t_size;
                uint32_t t_type, t_flags;
                lsp_trace("retrieve(%d (%s))", urid, unmap_urid(urid));
                const void *ptr   = hRetrieve(hHandle, urid, &t_size, &t_type, &t_flags);
                lsp_trace("retrieved ptr = %p, size=%d, type=%d, flags=0x%x", ptr, int(t_size), int(t_type), int(t_flags));
                if (ptr == NULL)
                    return NULL;

                lsp_trace("retrieved type = %d (%s)", int(t_type), unmap_urid(t_type));
                if (t_type != type)
                    return NULL;

                *size    = t_size;
                return ptr;
            }

            inline void reset_state_context()
            {
                hStore          = NULL;
                hRetrieve       = NULL;
                hHandle         = NULL;
            }

            inline bool ui_create_atom_transport(size_t port, size_t buf_size)
            {
                // Remember IDs of atom ports
                nAtomOut    = port++;
                nAtomIn     = port;

                // Allocate buffer
                nBufSize    = buf_size;
                pBuffer     = new uint8_t[nBufSize];
                if (pBuffer == NULL)
                    return false;

                lsp_trace("Atom rx_id=%d, tx_id=%d, buf_size=%d, buffer=%p", int(nAtomIn), int(nAtomOut), int(nBufSize), pBuffer);
                return true;
            }

            inline bool ui_connect_to_plugin()
            {
                if (map == NULL)
                    return false;

                // Prepare ofrge for transfer
                LV2_Atom_Forge_Frame    frame;
                forge_set_buffer(pBuffer, nBufSize);

                // Send CONNECT UI message
                lsp_trace("Sending CONNECT UI message");
                LV2_Atom *msg = forge_object(&frame, uridConnectUI, uridUINotification);
                forge_pop(&frame);
                write_data(nAtomOut, lv2_atom_total_size(msg), uridEventTransfer, msg);

                // Send PATCH GET message
                lsp_trace("Sending PATCH GET message");
                msg = forge_object(&frame, uridChunk, uridPatchGet);
                forge_pop(&frame);
                write_data(nAtomOut, lv2_atom_total_size(msg), uridEventTransfer, msg);

                // Sent STATE REQUEST message
                lsp_trace("Sending STATE REQUEST message");
                msg = forge_object(&frame, uridState, uridStateRequest);
                forge_pop(&frame);
                write_data(nAtomOut, lv2_atom_total_size(msg), uridEventTransfer, msg);

                lsp_trace("patch request has been written");
                return true;
            }

            inline void ui_disconnect_from_plugin()
            {
                // Prepare ofrge for transfer
                LV2_Atom_Forge_Frame    frame;
                forge_set_buffer(pBuffer, nBufSize);

                // Send DISCONNECT UI message
                lsp_trace("Sending DISCONNECT UI message");
                LV2_Atom *msg = forge_object(&frame, uridDisconnectUI, uridUINotification);
                forge_pop(&frame);
                write_data(nAtomOut, lv2_atom_total_size(msg), uridEventTransfer, msg);
            }

            inline bool ui_write_patch(LV2Serializable *p)
            {
                if ((map == NULL) || (p->get_urid() <= 0))
                    return false;

                // Forge PATCH SET message
                LV2_Atom_Forge_Frame    frame;
                forge_set_buffer(pBuffer, nBufSize);

                forge_frame_time(0);
                LV2_Atom *msg = forge_object(&frame, uridChunk, uridPatchSet);
                forge_key(uridPatchProperty);
                forge_urid(p->get_urid());
                forge_key(uridPatchValue);
                p->serialize();
                forge_pop(&frame);

                write_data(nAtomOut, lv2_atom_total_size(msg), uridEventTransfer, msg);
                return true;
            }

            inline bool ui_write_state(LV2Serializable *p)
            {
                if ((map == NULL) || (p->get_urid() <= 0))
                    return false;

                // Forge PATCH SET message
                LV2_Atom_Forge_Frame    frame;
                forge_set_buffer(pBuffer, nBufSize);

                forge_frame_time(0);
                LV2_Atom *msg = forge_object(&frame, uridState, uridStateChange);
                forge_key(p->get_urid());
                p->serialize();
                forge_pop(&frame);

                write_data(nAtomOut, lv2_atom_total_size(msg), uridEventTransfer, msg);
                return true;
            }
    };

    typedef struct LV2Mesh
    {
        size_t                  nMaxItems;
        size_t                  nBuffers;
        LV2_URID                uridItems;
        LV2_URID                uridDimensions;
        mesh_t                 *pMesh;
        LV2_URID               *pUrids;
        uint8_t                *pData;

        LV2Mesh()
        {
            nMaxItems       = 0;
            nBuffers        = 0;
            pMesh           = NULL;
            pUrids          = NULL;
            pData           = NULL;
            uridItems       = 0;
            uridDimensions  = 0;
        }

        ~LV2Mesh()
        {
            // Simply delete root structure
            if (pData != NULL)
            {
                delete [] (pData);
                pData       = NULL;
            }
            pUrids      = NULL;
            pMesh       = NULL;
        }

        void init(const port_t *meta, LV2Extensions *ext)
        {
            // Calculate sizes
            nBuffers            = meta->step;
            nMaxItems           = meta->start;

            size_t hdr_size     = ALIGN_SIZE(sizeof(mesh_t) + sizeof(float *) * nBuffers, DEFAULT_ALIGN);
            size_t urid_size    = ALIGN_SIZE(sizeof(LV2_URID) * nBuffers, DEFAULT_ALIGN);
            size_t buf_size     = ALIGN_SIZE(sizeof(float) * nMaxItems, DEFAULT_ALIGN);
            size_t to_alloc     = hdr_size + urid_size + buf_size * nBuffers;

            lsp_trace("buffers = %d, max_items=%d, hdr_size=%d, urid_size=%d, buf_size=%d, to_alloc=%d",
                    int(nBuffers), int(nMaxItems), int(hdr_size), int(urid_size), int(buf_size), int(to_alloc));
            pData               = new uint8_t[to_alloc + DEFAULT_ALIGN];
            if (pData == NULL)
                return;
            uint8_t *ptr        = ALIGN_PTR(pData, DEFAULT_ALIGN);
            pMesh               = reinterpret_cast<mesh_t *>(ptr);
            ptr                += hdr_size;
            pUrids              = reinterpret_cast<LV2_URID *>(ptr);
            ptr                += urid_size;

            lsp_trace("ptr = %p, pMesh = %p, pUrids = %p", ptr, pMesh, pUrids);

            for (size_t i=0; i<nBuffers; ++i)
            {
                lsp_trace("bufs[%d] = %p", int(i), ptr);
                pMesh->pvData[i]    = reinterpret_cast<float *>(ptr);
                ptr                += buf_size;
                pUrids[i]           = ext->map_uri("%s/Mesh#dimension%d", LSP_TYPE_URI(lv2), int(i));
            }

            lsp_assert(ptr > &pData[to_alloc + DEFAULT_ALIGN]);

            pMesh->nState       = M_WAIT;
            pMesh->nBuffers     = 0;
            pMesh->nItems       = 0;
            uridItems           = ext->map_uri("%s/Mesh#items", LSP_TYPE_URI(lv2));
            uridDimensions      = ext->map_uri("%s/Mesh#dimensions", LSP_TYPE_URI(lv2));

            lsp_trace("Initialized");
        }

        static size_t size_of_port(const port_t *meta)
        {
            size_t hdr_size     = sizeof(LV2_Atom_Int) + sizeof(LV2_Atom_Int) + 0x100; // Some extra bytes
            size_t prop_size    = sizeof(uint32_t) * 2;
            size_t vector_size  = prop_size + sizeof(LV2_Atom_Vector) + meta->start * sizeof(float);

            return LSP_LV2_SIZE_PAD(size_t(hdr_size + vector_size * meta->step));
        }
    } LV2Mesh;

    #define PATCH_OVERHEAD  (sizeof(LV2_Atom_Property) + sizeof(LV2_Atom_URID) + sizeof(LV2_Atom) + 0x20)
    #define STATE_OVERHEAD  (sizeof(LV2_Atom_Object) + sizeof(LV2_Atom_Property) + sizeof(LV2_Atom_Float) + 0x20)

    inline long lv2_all_port_sizes(const port_t *ports, bool in, bool out)
    {
        long size           = 0;

        for (const port_t *p = ports; (p->id != NULL) && (p->name != NULL); ++p)
        {
//            if (p->role != R_PORT_SET)
//            {
//                if (IS_OUT_PORT(p) && (!out))
//                    continue;
//                else if (IS_IN_PORT(p) && (!in))
//                    continue;
//            }

            switch (p->role)
            {
                case R_CONTROL:
                case R_METER:
                    size            += STATE_OVERHEAD + sizeof(LV2_Atom_Float);
                    break;
                case R_MESH:
                    if (IS_OUT_PORT(p) && (!out))
                        break;
                    else if (IS_IN_PORT(p) && (!in))
                        break;
                    size            += LV2Mesh::size_of_port(p);
                    break;
//                case R_MIDI:
//                    if (IS_OUT_PORT(p) && (!out))
//                        break;
//                    else if (IS_IN_PORT(p) && (!in))
//                        break;
//                    size            += (sizeof(LV2_Atom_Event) + 0x10) * MIDI_EVENTS_MAX; // Size of atom event + pad for MIDI data
//                    break;
                case R_PATH: // Both sizes: IN and OUT
                    size            += PATCH_OVERHEAD + PATH_MAX;
                    break;
                case R_PORT_SET:
                    if ((p->members != NULL) && (p->items != NULL))
                    {
                        size_t items        = list_size(p->items);
                        size               += items * lv2_all_port_sizes(p->members, in, out); // Add some overhead
                        size               += sizeof(LV2_Atom_Int) + 0x10;
                    }
                    break;
                default:
                    break;
            }
        }

        // Update state size
        return LSP_LV2_SIZE_PAD(size); // Add some extra bytes for
    }

    #undef PATCH_OVERHEAD
}


#endif /* CORE_CONTAINER_LV2EXT_H_ */
