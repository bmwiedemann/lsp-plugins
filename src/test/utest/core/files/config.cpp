/*
 * config.cpp
 *
 *  Created on: 15 мар. 2019 г.
 *      Author: sadko
 */

#include <test/utest.h>
#include <core/LSPString.h>
#include <core/files/config.h>
#include <core/io/NativeFile.h>
#include <metadata/metadata.h>

using namespace lsp;

UTEST_BEGIN("core.files", config)

    typedef struct tuple_t
    {
        LSPString key;
        LSPString value;
        LSPString comment;
        bool quoted;
        bool matched;
    } tuple_t;

    class TestConfigSource: public config::IConfigSource
    {
        private:
            cvector<tuple_t> *pTuples;
            size_t nID;

        public:
            explicit TestConfigSource(cvector<tuple_t> *vt) { pTuples = vt; nID = 0; }

            virtual ~TestConfigSource() { pTuples = NULL; }

        public:
            virtual status_t get_head_comment(LSPString *comment)
            {
                if (comment != NULL)
                {
                    LSPString c;

                    TEST_ASSERT(c.append_utf8("This file contains global configuration of plugins.\n"));
                    TEST_ASSERT(c.append('\n'));
                    TEST_ASSERT(c.append_utf8("(C) " LSP_FULL_NAME " \n"));
                    TEST_ASSERT(c.append_utf8("  " LSP_BASE_URI " \n"));

                    comment->take(&c);
                }
                return STATUS_OK;
            }

            virtual status_t get_parameter(LSPString *name, LSPString *value, LSPString *comment, int *flags)
            {
                if (nID >= pTuples->size())
                    return STATUS_EOF;

                tuple_t *t = pTuples->get(nID++);
                if (name != NULL)
                    TEST_ASSERT(name->set(&t->key));
                if (value != NULL)
                    TEST_ASSERT(value->set(&t->value));
                if (comment != NULL)
                    TEST_ASSERT(comment->set(&t->comment));
                if (flags != NULL)
                    *flags = (t->quoted) ? config::SF_QUOTED : config::SF_NONE;

                return STATUS_OK;
            }

            virtual status_t get_parameter(LSPString *name, LSPString *value, int *flags)
            {
                if (nID >= pTuples->size())
                    return STATUS_EOF;

                tuple_t *t = pTuples->get(nID++);
                if (name != NULL)
                    TEST_ASSERT(name->set(&t->key));
                if (value != NULL)
                    TEST_ASSERT(value->set(&t->value));
                if (flags != NULL)
                    *flags = (t->quoted) ? config::SF_QUOTED : config::SF_NONE;

                return STATUS_OK;
            }
    };

    class TestConfigHandler: public config::IConfigHandler
    {
        private:
            cvector<tuple_t> *pTuples;
            size_t nID;

        public:
            explicit TestConfigHandler(cvector<tuple_t> *vt) { pTuples = vt; nID = 0; }

            virtual ~TestConfigHandler() { pTuples = NULL; }

        public:
            virtual status_t handle_parameter(const LSPString *name, const LSPString *value)
            {
                TEST_ASSERT(name != NULL);
                TEST_ASSERT(value != NULL);

                for (size_t i=0, n=pTuples->size(); i<n; ++i)
                {
                    tuple_t *t = pTuples->get(i);
                    if (!t->key.equals(name))
                        continue;

                    TEST_ASSERT(t->value.equals(value));
                    TEST_ASSERT(!t->matched);
                    t->matched = true;
                    break;
                }

                return STATUS_OK;
            }

            virtual status_t handle_parameter(const char *name, const char *value)
            {
                LSPString xname, xvalue;
                TEST_ASSERT(name != NULL);
                TEST_ASSERT(value != NULL);
                TEST_ASSERT(xname.set_utf8(name));
                TEST_ASSERT(xname.set_utf8(value));

                return handle_parameter(&xname, &xvalue);
            }
    };

    void add_param(cvector<tuple_t> &vt, const char *key, const char *value, const char *comment, bool quoted)
    {
        tuple_t *t = new tuple_t;
        UTEST_ASSERT(t != NULL);
        UTEST_ASSERT(t->key.set_utf8(key));
        UTEST_ASSERT(t->value.set_utf8(value));
        UTEST_ASSERT(t->comment.set_utf8(comment));
        t->quoted = quoted;
        t->matched = false;
        UTEST_ASSERT(vt.add(t));
    }

    UTEST_MAIN
    {
        // Create tuples
        cvector<tuple_t> tuples;
        add_param(tuples, "mount_stud", "true", "Visibility of mount studs in the UI [boolean]: true/false", false);
        add_param(tuples, "version", LSP_MAIN_VERSION, "Last version of the product installed [pathname]", true);
        add_param(tuples, "dlg_sample_path", "/path/sample", "Dialog path for selecting sample files [pathname]", true);
        add_param(tuples, "dlg_ir_path", "/path/ir", "Dialog path for selecting impulse response files [pathname]", true);
        add_param(tuples, "dlg_config_path", "/path/to/config", "Dialog path for saving/loading configuration files [pathname]", true);
        add_param(tuples, "dlg_default_path", "/path/by/default", "Dialog default path for other files [pathname]", true);

        // Create file
        LSPString path;
        io::NativeFile fd;
        TestConfigSource cfg_src(&tuples);
        TestConfigHandler cfg_hdlr(&tuples);

        path.fmt_utf8("tmp/utest-%s.config.tmp", full_name());

        UTEST_ASSERT(fd.open(&path, io::File::FM_CREATE | io::File::FM_TRUNC | io::File::FM_READWRITE) == STATUS_OK);
        UTEST_ASSERT(config::save(&fd, &cfg_src, true) == STATUS_OK);
        UTEST_ASSERT(fd.size() > 0);
        UTEST_ASSERT(fd.seek(0, io::File::FSK_SET) == STATUS_OK);
        UTEST_ASSERT(config::load(&fd, &cfg_hdlr) == STATUS_OK);
        UTEST_ASSERT(fd.close() == STATUS_OK);

        // Destroy tuples
        for (size_t i=0,n=tuples.size(); i<n; ++i)
        {
            tuple_t *t = tuples.get(i);
            UTEST_ASSERT_MSG(t->matched, "Parameter %s was not deserialized", t->key.get_native());
            delete t;
        }
        tuples.flush();
    }

UTEST_END

