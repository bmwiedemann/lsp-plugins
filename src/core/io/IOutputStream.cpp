/*
 * IOutputStream.cpp
 *
 *  Created on: 14 нояб. 2017 г.
 *      Author: sadko
 */

#include <core/status.h>
#include <core/io/IOutputStream.h>

namespace lsp
{
    namespace io
    {
        
        IOutputStream::IOutputStream()
        {
            nErrorCode      = STATUS_OK;
        }
        
        IOutputStream::~IOutputStream()
        {
            nErrorCode      = STATUS_OK;
        }

        wssize_t IOutputStream::position()
        {
            return - (nErrorCode = STATUS_NOT_IMPLEMENTED);
        }

        ssize_t IOutputStream::write(const void *buf, size_t count)
        {
            return - (nErrorCode = STATUS_NOT_IMPLEMENTED);
        }

        wssize_t IOutputStream::seek(wsize_t position)
        {
            return - (nErrorCode = STATUS_NOT_IMPLEMENTED);
        }

        status_t IOutputStream::close()
        {
            return nErrorCode      = STATUS_OK;
        }
    
    } /* namespace ws */
} /* namespace lsp */
