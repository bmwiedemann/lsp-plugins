/*
 * ObjFileParser.cpp
 *
 *  Created on: 21 апр. 2017 г.
 *      Author: sadko
 */

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <locale.h>

#include <core/debug.h>
#include <core/files/3d/ObjFileParser.h>

#define IO_BUF_SIZE             8192

namespace lsp
{
    inline bool ObjFileParser::is_space(char ch)
    {
        return (ch == ' ') || (ch == '\t');
    }

    inline bool ObjFileParser::prefix_match(const char *s, const char *prefix)
    {
        while (*prefix != '\0')
        {
            if (*(s++) != *(prefix++))
                return false;
        }
        return is_space(*s);
    }

    bool ObjFileParser::parse_float(float *dst, const char **s)
    {
        if (*s == NULL)
            return false;

        errno = 0;
        char *ptr = NULL;
        float result = strtof(*s, &ptr);
        if ((errno != 0) || (ptr == *s))
            return false;
        *dst    = result;
        *s      = ptr;
        return true;
    }

    bool ObjFileParser::parse_int(ssize_t *dst, const char **s)
    {
        if (*s == NULL)
            return false;

        errno = 0;
        char *ptr = NULL;
        long result = strtol(*s, &ptr, 10);
        if ((errno != 0) || (ptr == *s))
            return false;
        *dst    = result;
        *s      = ptr;
        return true;
    }

    const char *ObjFileParser::skip_spaces(const char *s)
    {
        if (s == NULL)
            return NULL;

        while (true)
        {
            char ch = *s;
            if ((ch == '\0') || (!is_space(ch)))
                return s;
            s++;
        }
    }

    bool ObjFileParser::end_of_line(const char *s)
    {
        if (s == NULL)
            return true;

        while (true)
        {
            char ch = *s;
            if (ch == '\0')
                return true;
            if (!is_space(ch))
                return false;
            s++;
        }
    }

    void ObjFileParser::eliminate_comments(buffer_t *l)
    {
        char *p = l->pString;

        while (true)
        {
            char c = *p;
            if (c == '\0')
                return;
            else if (c == '#')
            {
                // Check that the comment is not back-slashed
                if ((p == l->pString) || (p[-1] != '\\'))
                {
                    while ((p > l->pString) && (is_space(p[-1])))
                        --p;

                    *p = '\0';
                    l->nLength  = p - l->pString;
                    return;
                }
                // move data
                memmove(p, p+1, (l->nLength--) - (p - l->pString));
            }

            p++;
        }
    }

    status_t ObjFileParser::read_line(file_buffer_t *fb)
    {
        clear_buf(&fb->line);

        while (true)
        {
            // Ensure that there is data in buffer
            if (fb->off >= fb->len)
            {
                fb->len = fread(fb->data, 1, IO_BUF_SIZE, fb->fd);
                if (fb->len <= 0)
                    return (feof(fb->fd)) ? STATUS_EOF : STATUS_IO_ERROR;
                fb->off     = 0;
            }

            // Skip windows '\r' character
            char *head  = &fb->data[fb->off];

            if (fb->skip_wc)
            {
                fb->skip_wc     = false;
                if (*head == '\r')
                {
                    fb->off++;
                    continue;
                }
            }

            // Scan for end-of-line
            char *eol   = reinterpret_cast<char *>(memchr(head, '\n', fb->len - fb->off));

            // Check that end-of-line was found
            if (eol != NULL)
            {
                fb->off    += eol - head + 1;
                if ((eol > head) && (eol[-1] == '\r'))
                    eol--;

                if (!append_buf(&fb->line, head, eol - head))
                    return STATUS_NO_MEM;

                // Check for special case
                if ((fb->line.nLength > 0) && (fb->line.pString[fb->line.nLength - 1] == '\\'))
                    fb->line.pString[--fb->line.nLength]    = '\0';
                else
                {
                    // Eliminate comments
                    eliminate_comments(&fb->line);
                    fb->skip_wc     = true;
                    return STATUS_OK;
                }
            }
            else
            {
                if (!append_buf(&fb->line, head, fb->len - fb->off))
                    return STATUS_NO_MEM;
                fb->off     = fb->len;
            }
        }
    }

    status_t ObjFileParser::parse_lines(file_buffer_t *fb, IFileHandler3D *handler)
    {
        status_t result = STATUS_OK;

        parse_state_t state;
        state.pHandler      = handler;
        state.nObjectID     = -1;
        state.nPointID      = 0;
        state.nFaceID       = 0;
        state.nLineID       = 0;
        state.nLines        = 0;

        state.nVxID         = 0;
        state.nParmVxID     = 0;
        state.nNormID       = 0;
        state.nTexVxID      = 0;

        while (true)
        {
            // Try to read line
            result = read_line(fb);
            if (result != STATUS_OK)
            {
                if (result == STATUS_EOF)
                    result      = parse_finish(&state);
                break;
            }

            // Check that line is not empty
            const char *l = skip_spaces(fb->line.pString);
            if (*l == '\0')
                continue;

            // Parse line
            result = parse_line(&state, fb->line.pString);
            if (result != STATUS_OK)
                break;
        }

        // Destroy state
        state.sVx.flush();
        state.sParVx.flush();
        state.sTexVx.flush();
        state.sNorm.flush();

        state.sVxIdx.flush();
        state.sTexVxIdx.flush();
        state.sNormIdx.flush();

        return result;
    }

    status_t ObjFileParser::parse(const char *path, IFileHandler3D *handler)
    {
        // Try to open file
        errno       = 0;
        FILE *fd    = fopen(path, "rb");
        if (fd == NULL)
        {
            if (errno == EPERM)
                return STATUS_PERMISSION_DENIED;
            else if (errno == ENOENT)
                return STATUS_NOT_FOUND;
            return STATUS_IO_ERROR;
        }

        // Initialize file buffer
        file_buffer_t fb;
        fb.fd       = fd;
        fb.data     = new char[IO_BUF_SIZE];
        if (fb.data == NULL)
        {
            fclose(fd);
            return STATUS_NO_MEM;
        }
        init_buf(&fb.line); // Initialize line buffer
        fb.len      = 0;
        fb.off      = 0;
        fb.skip_wc  = false;

        char *saved_locale = setlocale(LC_NUMERIC, "C");
        status_t result     = parse_lines(&fb, handler);
        setlocale(LC_NUMERIC, saved_locale);

        // Delete all allocated data
        destroy_buf(&fb.line);
        delete [] fb.data;
        fclose(fd);

        return result;
    }

    status_t ObjFileParser::parse_line(parse_state_t *st, const char *s)
    {
        lsp_trace("%s", s);
        status_t result = ((st->nLines++) > 0) ? STATUS_CORRUPTED_FILE : STATUS_BAD_FORMAT;

        switch (*(s++))
        {
            case 'b': // bmat, bevel
                if (prefix_match(s, "mat")) // bmat
                    return STATUS_OK;
                else if (prefix_match(s, "evel")) // bevel
                    return STATUS_OK;
                break;

            case 'c': // cstype, curv, curv2, con, c_interp, ctech
                if (prefix_match(s, "stype")) // cstype
                    return STATUS_OK;
                else if (prefix_match(s, "urv")) // curv
                    return STATUS_OK;
                else if (prefix_match(s, "urv2")) // curv2
                    return STATUS_OK;
                else if (prefix_match(s, "on")) // con
                    return STATUS_OK;
                else if (prefix_match(s, "_interp")) // c_interp
                    return STATUS_OK;
                else if (prefix_match(s, "tech")) // ctech
                    return STATUS_OK;
                break;

            case 'd': // deg, d_interp
                if (prefix_match(s, "eg")) // deg
                    return STATUS_OK;
                else if (prefix_match(s, "_interp")) // d_interp
                    return STATUS_OK;
                break;

            case 'e': // end
                if (prefix_match(s, "nd")) // end
                    return STATUS_OK;
                break;

            case 'f': // f
                if (is_space(*s)) // f
                {
                    // Clear previously used lists
                    st->sVxIdx.clear();
                    st->sTexVxIdx.clear();
                    st->sNormIdx.clear();

                    // Parse face
                    while (true)
                    {
                        ssize_t v = 0, vt = 0, vn = 0;

                        // Parse indexes
                        s   = skip_spaces(s);
                        if (!parse_int(&v, &s))
                            break;
                        if (*(s++) != '/')
                            return result;
                        if (!parse_int(&vt, &s))
                            vt  = 0;
                        if (*(s++) != '/')
                            return result;
                        if (!parse_int(&vn, &s))
                            vn = 0;

                        // Ensure that indexes are correct
                        v   = (v < 0) ? st->sVx.size() + v : v - 1;
                        if ((v < 0) || (v >= ssize_t(st->sVx.size())))
                            return result;

                        vt  = (vt < 0) ? st->sTexVx.size() + vt : vt - 1;
                        if ((vt < -1) || (vt >= ssize_t(st->sTexVx.size())))
                            return result;

                        vn  = (vn < 0) ? st->sNorm.size() + vn : vn - 1;
                        if ((vn < -1) || (vn >= ssize_t(st->sNorm.size())))
                            return result;

                        // Register vertex
                        ofp_point3d_t *xp = st->sVx.at(v);
                        if (xp->oid != st->nObjectID)
                        {
                            result  = st->pHandler->add_vertex(xp);
                            if (result != STATUS_OK)
                                return result;

                            xp->oid     = st->nObjectID;
                            xp->idx     = st->nVxID++;
                        }
                        v           = xp->idx;

                        // Register texture vertex
                        if (vt >= 0)
                        {
                            xp = st->sVx.at(vt);
                            if (xp->oid != st->nObjectID)
                            {
                                result      = st->pHandler->add_texture_vertex(xp);
                                if (result != STATUS_OK)
                                    return result;

                                xp->oid     = st->nObjectID;
                                xp->idx     = st->nTexVxID++;
                            }
                            vt  = xp->idx;
                        }

                        // Register normal vector
                        if (vn >= 0)
                        {
                            ofp_vector3d_t *xn = st->sNorm.at(vn);
                            if (xn->oid != st->nObjectID)
                            {
                                result      = st->pHandler->add_normal(xn);
                                if (result != STATUS_OK)
                                    return result;

                                xn->oid     = st->nObjectID;
                                xn->idx     = st->nNormID++;
                            }
                            vn  = xn->idx;
                        }

                        // Add items to lists
                        if (!st->sVxIdx.add(v))
                            return STATUS_NO_MEM;
                        if (!st->sTexVxIdx.add(vt))
                            return STATUS_NO_MEM;
                        if (!st->sNormIdx.add(vn))
                            return STATUS_NO_MEM;
                    }

                    if (!end_of_line(s))
                        return result;

                    // Check face parameters
                    if (st->sVxIdx.size() < 3)
                        return STATUS_BAD_FORMAT;

                    // Call parser to handle data
                    result = st->pHandler->add_face(st->sVxIdx.get_array(), st->sNormIdx.get_array(), st->sTexVxIdx.get_array(), st->sVxIdx.size());
                }
                break;

            case 'g': // g
                if (is_space(*s)) // g
                    return STATUS_OK;
                break;

            case 'h': // hole
                if (prefix_match(s, "ole")) // hole
                    return STATUS_OK;
                break;

            case 'l': // l, lod
                if (is_space(*s)) // l
                {
                    // Clear previously used lists
                    st->sVxIdx.clear();
                    st->sTexVxIdx.clear();

                    // Parse face
                    while (true)
                    {
                        ssize_t v = 0, vt = 0;

                        // Parse indexes
                        s   = skip_spaces(s);
                        if (!parse_int(&v, &s))
                            break;
                        if (*(s++) != '/')
                            return result;
                        if (!parse_int(&vt, &s))
                            vt = 0;

                        // Ensure that indexes are correct
                        v   = (v < 0) ? st->sVx.size() + v : v - 1;
                        if ((v < 0) || (v >= ssize_t(st->sVx.size())))
                            return result;

                        vt  = (vt < 0) ? st->sTexVx.size() + vt : vt - 1;
                        if ((vt <= -1) || (vt >= ssize_t(st->sTexVx.size())))
                            return result;

                        // Register vertex
                        ofp_point3d_t *xp   = st->sVx.at(v);
                        if (xp->oid != st->nObjectID)
                        {
                            result  = st->pHandler->add_vertex(xp);
                            if (result != STATUS_OK)
                                return result;

                            xp->oid     = st->nObjectID;
                            xp->idx     = st->nVxID++;
                        }
                        v           = xp->idx;

                        // Register texture vertex
                        if (vt >= 0)
                        {
                            xp = st->sVx.at(vt);
                            if (xp->oid != st->nObjectID)
                            {
                                result      = st->pHandler->add_texture_vertex(xp);
                                if (result != STATUS_OK)
                                    return result;

                                xp->oid     = st->nObjectID;
                                xp->idx     = st->nTexVxID++;
                            }
                            vt  = xp->idx;
                        }

                        // Add items to lists
                        if (!st->sVxIdx.add(&v))
                            return STATUS_NO_MEM;
                        if (!st->sTexVxIdx.add(&vt))
                            return STATUS_NO_MEM;
                    }

                    if (!end_of_line(s))
                        return result;

                    // Check face parameters
                    if (st->sVxIdx.size() < 3)
                        return STATUS_BAD_FORMAT;

                    // Call parser to handle data
                    result = st->pHandler->add_line(st->sVxIdx.get_array(), st->sTexVxIdx.get_array(), st->sVxIdx.size());
                }
                else if (prefix_match(s, "od")) // lod
                    return STATUS_OK;
                break;

            case 'm': // mg, mtllib
                if (prefix_match(s, "g")) // mg
                    return STATUS_OK;
                else if (prefix_match(s, "tllib")) // mtllib
                    return STATUS_OK;
                break;

            case 'o': // o
                if (is_space(*s)) // o
                {
                    s   = skip_spaces(s+1);
                    if (st->nObjectID >= 0)
                    {
                        result = st->pHandler->end_object(st->nObjectID);
                        if (result != STATUS_OK)
                            return result;
                    }
                    result = st->pHandler->begin_object(++st->nObjectID, s);
                    st->nVxID           = 0;
                    st->nParmVxID       = 0;
                    st->nNormID         = 0;
                    st->nTexVxID        = 0;
                }
                break;

            case 'p': // p, parm
                if (is_space(*s)) // p
                {
                    st->sVxIdx.clear();

                    // Parse point
                    while (true)
                    {
                        ssize_t v = 0;

                        // Parse indexes
                        s   = skip_spaces(s);
                        if (!parse_int(&v, &s))
                            break;

                        // Ensure that indexes are correct
                        v   = (v < 0) ? st->nVxID + v : v - 1;
                        if ((v < 0) || (v > ssize_t(st->nVxID)))
                            return result;

                        // Register vertex
                        ofp_point3d_t *xp   = st->sVx.at(v);
                        if (xp->oid != st->nObjectID)
                        {
                            result  = st->pHandler->add_vertex(xp);
                            if (result != STATUS_OK)
                                return result;

                            xp->oid     = st->nObjectID;
                            xp->idx     = st->nVxID++;
                        }
                        v           = xp->idx;

                        // Add items to lists
                        if (!st->sVxIdx.add(&v))
                            return STATUS_NO_MEM;
                    }

                    // Check that we reached end of line
                    if (!end_of_line(s))
                        return result;

                    result = st->pHandler->add_points(st->sVxIdx.get_array(), st->sVxIdx.size());
                }
                else if (prefix_match(s, "arm")) // parm
                    return STATUS_OK;
                break;

            case 's': // s, step, surf, scrv, sp, shadow_obj, stech
                if (is_space(*s)) // s
                    return STATUS_OK;
                else if (prefix_match(s, "tep")) // step
                    return STATUS_OK;
                else if (prefix_match(s, "urf")) // surf
                    return STATUS_OK;
                else if (prefix_match(s, "rcv")) // srcv
                    return STATUS_OK;
                else if (prefix_match(s, "p")) // sp
                    return STATUS_OK;
                else if (prefix_match(s, "hadow_obj")) // shadow_obj
                    return STATUS_OK;
                else if (prefix_match(s, "tech")) // stech
                    return STATUS_OK;
                break;

            case 't': // trim, trace_obj
                if (prefix_match(s, "rim")) // trim
                    return STATUS_OK;
                else if (prefix_match(s, "race_obj")) // trace_obj
                    return STATUS_OK;
                break;

            case 'u': // usemtl
                if (prefix_match(s, "semtl")) // usemtl
                    return STATUS_OK;
                break;

            case 'v': // v, vt, vn, vp
                if (is_space(*s)) // v
                {
                    ofp_point3d_t p;

                    s   = skip_spaces(s+1);
                    if (!parse_float(&p.x, &s))
                        return result;
                    s   = skip_spaces(s);
                    if (!parse_float(&p.y, &s))
                        return result;
                    s   = skip_spaces(s);
                    if (!parse_float(&p.z, &s))
                        p.z     = 0.0f; // Extension, strictly required in obj format, for our case facilitated
                    s   = skip_spaces(s);
                    if (!parse_float(&p.w, &s))
                        p.w     = 1.0f;

                    if (!end_of_line(s))
                        return result;

                    p.oid       = -1;
                    p.idx       = -1;
                    if (!st->sVx.add(&p))
                        return STATUS_NO_MEM;
                    result = STATUS_OK;
                }
                else if (prefix_match(s, "n")) // vn
                {
                    ofp_vector3d_t v;

                    s   = skip_spaces(s+2);
                    if (!parse_float(&v.dx, &s))
                        return result;
                    s   = skip_spaces(s);
                    if (!parse_float(&v.dy, &s))
                        return result;
                    s   = skip_spaces(s);
                    if (!parse_float(&v.dz, &s))
                        v.dz    = 0.0f; // Extension, strictly required in obj format, for our case facilitated
                    v.dw    = 0.0f;

                    if (!end_of_line(s))
                        return result;

                    v.oid       = -1;
                    v.idx       = -1;
                    if (!st->sNorm.add(&v))
                        return STATUS_NO_MEM;
                    result = STATUS_OK;
                }
                else if (prefix_match(s, "p")) // vp
                {
                    ofp_point3d_t p;

                    s   = skip_spaces(s+2);
                    if (parse_float(&p.x, &s))
                        return result;
                    s   = skip_spaces(s);
                    if (!parse_float(&p.y, &s))
                        p.y     = 0.0f;
                    p.z     = 0.0f;
                    s   = skip_spaces(s);
                    if (!parse_float(&p.w, &s))
                        p.w     = 1.0f;

                    if (!end_of_line(s))
                        return result;

                    p.oid       = -1;
                    p.idx       = -1;
                    if (!st->sParVx.add(&p))
                        return STATUS_NO_MEM;
                    result = STATUS_OK;
                }
                else if (prefix_match(s, "t")) // vt
                {
                    ofp_point3d_t p;

                    s   = skip_spaces(s+2);
                    if (!parse_float(&p.x, &s))
                        return result;
                    s   = skip_spaces(s);
                    if (!parse_float(&p.y, &s))
                        p.y     = 0.0f;
                    p.z     = 0.0f;
                    s   = skip_spaces(s);
                    if (!parse_float(&p.w, &s))
                        p.w     = 0.0f;

                    if (!end_of_line(s))
                        return result;

                    p.oid       = -1;
                    p.idx       = -1;
                    if (!st->sTexVx.add(&p))
                        return STATUS_NO_MEM;
                    result = STATUS_OK;
                }
                break;
        }

        return result;
    }

    status_t ObjFileParser::parse_finish(parse_state_t *st)
    {
        status_t result = STATUS_OK;

        if (st->nObjectID >= 0)
        {
            result = st->pHandler->end_object(st->nObjectID);
            if (result != STATUS_OK)
                return result;
        }

        return result;
    }
} /* namespace lsp */
