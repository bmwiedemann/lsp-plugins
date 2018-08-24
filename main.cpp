#include "test/common.h"

#include "test/res_gen.h"
#include "test/audiofile_test.h"
#include "test/convolver_test.h"
#include "test/window_test.h"
#include "test/env_test.h"
#include "test/frac_test.h"
#include "test/sample_player_test.h"
#include "test/randgen_test.h"
#include "test/fade_test.h"
#include "test/resample_test.h"
#include "test/genttl.h"
#include "test/alloc_test.h"
#include "test/jack_test.h"

#include "test/fft_test.h"
#include "test/fft_test2.h"
#include "test/fft_test3.h"
#include "test/fft_speed_test.h"
#include "test/fft_conv_test.h"
#include "test/fft_fastconv_test.h"
#include "test/fft_fastconv_speed_test.h"
#include "test/fft_tdomain_test.h"
#include "test/fft_filter.h"
#include "test/fft_join.h"

#include "test/real_to_complex.h"

#include "test/saturation_test.h"
#include "test/addm_test.h"
#include "test/equalizer_test.h"
#include "test/sidechain_test.h"
#include "test/limiter_test.h"

#include "test/files/config_test.h"
#include "test/files/text_read_test.h"

#include "test/sse_test.h"

#include "test/oversampler_test.h"

//#include "test/geometry3d_test.h"
//#include "test/objfile3d_test.h"
//#include "test/anyfile3d_test.h"

#include "test/latencydetector_test.h"
#include "test/lspstring.h"
#include "test/lspc_test.h"
#include "test/clipbrd_test.h"
#include "test/solutions3d_test.h"
#include "test/synchronizedchirp_test.h"

#include "test/algo/bt_test.h"
#include "test/algo/mt_test.h"

#include "test/math/root_test.h"

#include "test/x11/selection_test.h"


//#define TEST config_test
//#define TEST text_read_test

//#define TEST xwindow_test
//#define TEST limiter_test
//#define TEST fft_conv_test
//#define TEST crossover_test
//#define TEST saturation_test
//#define TEST log_test
//#define TEST addm_test
//#define TEST filter_test
//#define TEST dyn_filter_test
//#define TEST equalizer_test
//#define TEST avx_test
//#define TEST sidechain_test

//#define TEST ladspa_test
//#define TEST vst_test
//#define TEST res_gen
//#define TEST audiofile_test
//#define TEST convolver_test

//#define TEST fft_test
//#define TEST fft_test2
//#define TEST fft_test3
//#define TEST fft_tdomain_test
//#define TEST fft_fastconv_test
//#define TEST fft_fastconv_speed_test
//#define TEST fft_filter_test
//#define TEST fft_speed_test
//#define TEST fft_join_test

//#define TEST complex_mul_speed_test
//#define TEST real_to_complex_test

//#define TEST dsp_speed_test
//#define TEST window_test
//#define TEST env_test
//#define TEST sample_player_test
//#define TEST frac_test
//#define TEST randgen_test
//#define TEST fade_test
//#define TEST resample_test
//#define TEST genttl_test
//#define TEST lv2bufsize_test
//#define TEST alloc_test
//#define TEST profiling_test
//#define TEST lspstring_test
//#define TEST clipbrd_test

//#define TEST downsampling_test
//#define TEST oversampling_test
//#define TEST oversampler_test

//#define TEST sse_test
//#define TEST latencydetector_test
//#define TEST geometry3d_test
//#define TEST solutions3d_test
//#define TEST objfile3d_test
//#define TEST anyfile3d_test
//#define TEST synchronizedchirp_test
//#define TEST lspc_test

//#define TEST bt_test
//#define TEST mt_test
//#define TEST bt_perf_test
//#define TEST mt_perf_test
//#define TEST root_test

#define TEST jack_test
//#define TEST selection_test
//#define TEST endian_test

#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <alloca.h>
#include <signal.h>

#include <dsp/dsp.h>
#include <test/ptest.h>
#include <test/utest.h>
#include <data/cvector.h>
#include <core/LSPString.h>

enum test_mode_t
{
    UNKNOWN,
    PTEST,
    UTEST
};

using namespace lsp;

typedef struct config_t
{
    mode_t                      mode;
    bool                        fork;
    bool                        verbose;
    bool                        debug;
    bool                        list_all;
    size_t                      threads;
    cvector<LSPString>          list;
} config_t;

typedef struct stats_t
{
    size_t      total;
    size_t      success;
    double      overall;
    cvector<test::Test> failed; // List of failed tests
} stats_t;

typedef struct task_t
{
    pid_t               pid;
    struct timespec     submitted;
    test::UnitTest     *utest;
} task_t;

bool match_string(const LSPString *p, const LSPString *m)
{
    size_t m_off = 0;

    for (size_t off = 0, len = p->length(); off < len; )
    {
        // Get pattern token
        LSPString *pt = NULL;
        ssize_t next = p->index_of(off, '.');
        if (next < 0)
        {
            pt = p->substring(off);
            off = p->length();
        }
        else
        {
            pt = p->substring(off, next);
            off = next + 1;
        }

        // Get match token
        if (m_off >= m->length())
            return false;

        LSPString *mt = NULL;
        next = m->index_of(m_off, '.');
        if (next < 0)
        {
            mt = m->substring(m_off);
            m_off = m->length();
        }
        else
        {
            mt = m->substring(m_off, next);
            m_off = next + 1;
        }

        // Check wildcard
        if ((pt->length() == 1) && (pt->char_at(0) == '*'))
            continue;

        // Check equality
        if (!pt->equals(mt))
            return false;
    }

    return true;
}

bool match_list(lsp::cvector<LSPString> &list, const char *group, const char *name)
{
    // Empty list always matches
    if (list.size() <= 0)
        return true;

    LSPString m;
    m.set_ascii(group);
    m.append('.');
    m.append_ascii(name);

    for (size_t i=0, n=list.size(); i<n; ++i)
    {
        LSPString *p = list.at(i);
        if (p == NULL)
            continue;
        if (match_string(p, &m))
            return true;
    }

    return false;
}

int output_stats(stats_t *stats, const char *text)
{
    printf("\n--------------------------------------------------------------------------------\n");
    printf("%s:\n", text);
    printf("  overall time [s]:     %.2f\n", stats->overall);
    printf("  launched:             %d\n", int(stats->total));
    printf("  succeeded:            %d\n", int(stats->success));
    printf("  failed:               %d\n", int(stats->failed.size()));

    if (stats->failed.size() > 0)
    {
        printf("\n--------------------------------------------------------------------------------\n");
        printf("List of failed tests:\n");
        for (size_t i=0, n=stats->failed.size(); i<n; ++i)
        {
            test::Test *t = stats->failed.at(i);
            printf("  %s.%s\n", t->group(), t->name());
        }
        printf("\n");
        return 2;
    }

    printf("\n");
    return 0;
}

int execute_ptest(config_t *cfg, test::PerformanceTest *v)
{
    // Execute performance test
    v->set_verbose(cfg->verbose);
    v->execute();

    // Output peformance test statistics
    printf("\nStatistics of performance test '%s.%s':\n", v->group(), v->name());
    v->dump_stats();
    v->free_stats();
    return 0;
}

void utest_sighandler(int signum)
{
    fprintf(stderr, "Unit test time limit exceeded\n");
    exit(2);
}

int output_test_list(const char *text, cvector<char> &list)
{
    printf("\n%s:\n", text);
    size_t n = list.size();
    for (size_t i=0; i<n-1; ++i)
        for (size_t j=i+1; j<n; ++j)
            if (strcmp(list.at(i), list.at(j)) > 0)
                list.swap(i, j);

    for (size_t i=0; i<n; ++i)
    {
        char *name = list.at(i);
        printf("  %s\n", name);
        free(name);
    }
    printf("\n");
    list.flush();
    return 0;
}

int execute_utest(config_t *cfg, test::UnitTest *v)
{
    // Set-up timer for deadline expiration
    struct itimerval timer;

    if (!cfg->debug)
    {
        timer.it_interval.tv_sec    = v->time_limit();
        timer.it_interval.tv_usec   = suseconds_t(v->time_limit() * 1e+9) % 1000000000L;
        timer.it_value              = timer.it_interval;

        signal(SIGALRM, utest_sighandler);
        if (setitimer(ITIMER_REAL, &timer, NULL) != 0)
            exit(4);
    }

    // Execute performance test
    v->set_verbose(cfg->verbose);
    v->execute();

    // Cancel and disable timer
    if (!cfg->debug)
    {
        timer.it_interval.tv_sec    = 0;
        timer.it_interval.tv_usec   = 0;
        timer.it_value              = timer.it_interval;

        if (setitimer(ITIMER_REAL, &timer, NULL) != 0)
            exit(4);
        signal(SIGALRM, SIG_DFL);
    }

    // Return success
    return 0;
}

int launch_ptest(config_t *cfg)
{
    using namespace test;
    PerformanceTest *v = ptest_init();
    if (v == NULL)
    {
        fprintf(stderr, "No performance tests available\n");
        return -1;
    }

    // List all tests if requested
    if (cfg->list_all)
    {
        cvector<char> names;

        for ( ; v != NULL; v = v->next())
        {
            char *str = NULL;
            asprintf(&str, "%s.%s", v->group(), v->name());
            if (str != NULL)
            {
                if (!names.add(str))
                    free(str);
            }
        }
        return output_test_list("List of available performance tests", names);
    }

    int result = 0;
    struct timespec ts, start, finish;
    stats_t stats;
    stats.total     = 0;
    stats.success   = 0;
    stats.overall   = 0.0;
    double time     = 0.0;

    clock_gettime(CLOCK_REALTIME, &ts);

    for ( ; v != NULL; v = v->next())
    {
        // Check that test is not ignored
        if (v->ignore())
            continue;

        // Need to check test name and group?
        if (!match_list(cfg->list, v->group(), v->name()))
            continue;

        printf("\n--------------------------------------------------------------------------------\n");
        printf("Launching performance test '%s.%s'\n", v->group(), v->name());
        printf("--------------------------------------------------------------------------------\n");

        clock_gettime(CLOCK_REALTIME, &start);

        stats.total     ++;
        if (cfg->fork)
        {
            pid_t pid = fork();
            if (pid < 0)
            {
                int error = errno;
                fprintf(stderr, "Error while spawning child process %d\n", error);

                result = -2;
                break;
            }
            else if (pid == 0)
                return execute_ptest(cfg, v);
            else
            {
                // Parent process code: wait for nested process execution
                do
                {
                    int w = waitpid(pid, &result, WUNTRACED | WCONTINUED);
                    if (w < 0)
                    {
                        fprintf(stderr, "Waiting for performance test '%s.%s' failed\n", v->group(), v->name());
                        stats.failed.add(v);
                        break;
                    }

                    if (WIFEXITED(result))
                        printf("Performance test '%s.%s' finished, status=%d\n", v->group(), v->name(), WEXITSTATUS(result));
                    else if (WIFSIGNALED(result))
                        printf("Performance test '%s.%s' killed by signal %d\n", v->group(), v->name(), WTERMSIG(result));
                    else if (WIFSTOPPED(result))
                        printf("Performance test '%s.%s' stopped by signal %d\n", v->group(), v->name(), WSTOPSIG(result));
                } while (!WIFEXITED(result) && !WIFSIGNALED(result));
            }
        }
        else
            result = execute_ptest(cfg, v);

        clock_gettime(CLOCK_REALTIME, &finish);
        time = (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec) * 1e-9;

        printf("Performance test '%s.%s' execution time: %.2f s\n", v->group(), v->name() , time);
        if (result == 0)
            stats.success ++;
        else
            stats.failed.add(v);
    }

    clock_gettime(CLOCK_REALTIME, &finish);
    stats.overall = (finish.tv_sec - ts.tv_sec) + (finish.tv_nsec - ts.tv_nsec) * 1e-9;
    return output_stats(&stats, "Overall performance test statistics");
}

bool wait_thread(config_t *cfg, task_t *threads, stats_t *stats)
{
    struct timespec ts;
    int result = 0;
    pid_t pid = -1;
    task_t *t;

    do
    {
        pid = waitpid(-1, &result, WUNTRACED | WCONTINUED);
        if (pid < 0)
        {
            fprintf(stderr, "Waiting for unit test completion failed\n");
            exit(4);
            break;
        }

        // Find the associated unit test process
        t = NULL;
        for (size_t i=0; i<cfg->threads; i++)
            if (threads[i].pid == pid)
                t = &threads[i];
        if (t == NULL)
            return false;

        if (WIFEXITED(result))
            printf("Unit test '%s.%s' finished, status=%d\n", t->utest->group(), t->utest->name(), WEXITSTATUS(result));
        else if (WIFSIGNALED(result))
            printf("Unit test '%s.%s' killed by signal %d\n", t->utest->group(), t->utest->name(), WTERMSIG(result));
        else if (WIFSTOPPED(result))
            printf("Unit test '%s.%s' stopped by signal %d\n", t->utest->group(), t->utest->name(), WSTOPSIG(result));
    } while (!WIFEXITED(result) && !WIFSIGNALED(result));

    // Get time
    clock_gettime(CLOCK_REALTIME, &ts);
    double time = (ts.tv_sec - t->submitted.tv_sec) + (ts.tv_nsec - t->submitted.tv_nsec) * 1e-9;
    printf("Unit test '%s.%s' execution time: %.2f s\n", t->utest->group(), t->utest->name(), time);
    if (result == 0)
        stats->success ++;
    else
        stats->failed.add(t->utest);

    // Free task
    t->pid                  = -1;
    t->submitted.tv_sec     = 0;
    t->submitted.tv_nsec    = 0;
    t->utest                = NULL;

    return true;
}

int wait_threads(config_t *cfg, task_t *threads, stats_t *stats)
{
    // Estimate number of child processes
    size_t nwait = 0;
    for (size_t i=0; i<cfg->threads; i++)
        if (threads[i].pid >= 0)
            nwait++;

    // Wait until all children become completed
    while (nwait > 0)
    {
        if (wait_thread(cfg, threads, stats))
            nwait--;
    }

    return 0;
}

int submit_utest(config_t *cfg, task_t *threads, stats_t *stats, test::UnitTest *v)
{
    if (!cfg->fork)
    {
        struct timespec start, finish;
        clock_gettime(CLOCK_REALTIME, &start);

        int res     = execute_utest(cfg, v);

        clock_gettime(CLOCK_REALTIME, &finish);
        double time = (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec) * 1e-9;
        printf("Test execution time: %.2f s\n", time);

        if (res == 0)
            stats->success++;
        else
            stats->failed.add(v);
        return 0;
    }

    // Find placeholder
    while (true)
    {
        // Check that we are able to submit task
        task_t *t = NULL;
        for (size_t i=0; i<cfg->threads; i++)
            if (threads[i].pid < 0)
            {
                t = &threads[i];
                break;
            }

        // Is there a placeholder?
        if (t != NULL)
        {
            pid_t pid = fork();
            if (pid < 0)
            {
                int error = errno;
                fprintf(stderr, "Error while spawning child process %d\n", error);
                return -2;
            }
            else if (pid == 0)
            {
                int res = execute_utest(cfg, v);
                exit(res);
            }
            else
            {
                clock_gettime(CLOCK_REALTIME, &t->submitted);
                t->pid          = pid;
                t->utest        = v;
                return 0;
            }
        }
        else // Wait for child process completion
            wait_thread(cfg, threads, stats);
    }
}

int launch_utest(config_t *cfg)
{
    using namespace test;
    UnitTest *v = utest_init();
    if (v == NULL)
    {
        fprintf(stderr, "No performance tests available\n");
        return -1;
    }

    // List all tests if requested
    if (cfg->list_all)
    {
        cvector<char> names;

        for ( ; v != NULL; v = v->next())
        {
            char *str = NULL;
            asprintf(&str, "%s.%s", v->group(), v->name());
            if (str != NULL)
            {
                if (!names.add(str))
                    free(str);
            }
        }
        return output_test_list("List of available performance tests", names);
    }

    struct timespec start, finish;
    stats_t stats;

    clock_gettime(CLOCK_REALTIME, &start);
    stats.total     = 0;
    stats.success   = 0;

    task_t *threads = (cfg->fork) ? reinterpret_cast<task_t *>(alloca(sizeof(task_t) * cfg->threads)) : NULL;
    if (threads != NULL)
    {
        for (size_t i=0; i<cfg->threads; i++)
        {
            threads[i].pid              = -1;
            threads[i].submitted.tv_sec = 0;
            threads[i].submitted.tv_nsec= 0;
            threads[i].utest            = NULL;
        }
    }

    for ( ; v != NULL; v = v->next())
    {
        // Check that test is not ignored
        if (v->ignore())
            continue;

        // Need to check test name and group?
        if (!match_list(cfg->list, v->group(), v->name()))
            continue;

        printf("\n--------------------------------------------------------------------------------\n");
        printf("Launching unit test '%s.%s'\n", v->group(), v->name());
        printf("--------------------------------------------------------------------------------\n");

        stats.total ++;
        if (submit_utest(cfg, threads, &stats, v) != 0)
            break;
    }

    // Wait until all child processes exit
    if (threads != NULL)
    {
        if (wait_threads(cfg, threads, &stats) != 0)
            fprintf(stderr, "Error while waiting child processes for comletion");
    }

    clock_gettime(CLOCK_REALTIME, &finish);
    stats.overall = (finish.tv_sec - start.tv_sec) + (finish.tv_nsec - start.tv_nsec) * 1e-9;

    return output_stats(&stats, "Overall unit test statistics");
}

int usage()
{
    fprintf(stderr, "USAGE: {ptest|utest} [args...]\n");
    return 1;
}

int parse_config(config_t *cfg, int argc, const char **argv)
{
    cfg->mode       = UNKNOWN;
    cfg->fork       = true;
    cfg->verbose    = false;
    cfg->list_all   = false;
    cfg->threads    = sysconf(_SC_NPROCESSORS_ONLN) * 2;
    if (argc < 2)
        return usage();

    if (!strcmp(argv[1], "ptest"))
        cfg->mode = PTEST;
    else if (!strcmp(argv[1], "utest"))
        cfg->mode = UTEST;
    if (cfg->mode == UNKNOWN)
        return usage();

    for (int i=2; i<argc; ++i)
    {
        if (!strcmp(argv[i], "--nofork"))
            cfg->fork       = false;
        else if (!strcmp(argv[i], "--fork"))
            cfg->fork       = true;
        else if (!strcmp(argv[i], "--verbose"))
            cfg->verbose    = true;
        else if (!strcmp(argv[i], "--silent"))
            cfg->verbose    = false;
        else if (!strcmp(argv[i], "--debug"))
            cfg->debug      = true;
        else if (!strcmp(argv[i], "--list"))
            cfg->list_all   = true;
        else if (!strcmp(argv[i], "--threads"))
        {
            if ((++i) >= argc)
            {
                fprintf(stderr, "Not specified number of threads for --threads parameter\n");
                return 3;
            }

            errno           = 0;
            char *end       = NULL;
            cfg->threads    = strtol(argv[i], &end, 10);
            if ((errno != 0) || ((*end) != '\0'))
            {
                fprintf(stderr, "Invalid value for --threads parameter: %s\n", argv[i]);
                return 3;
            }
        }
        else
        {
            LSPString *s = new LSPString();
            s->set_native(argv[i]);
            cfg->list.add(s);
        }
    }

    return 0;
}

void clear_config(config_t *cfg)
{
    for (size_t i=0, n=cfg->list.size(); i<n; ++i)
        delete cfg->list.at(i);
    cfg->list.clear();
}

int main(int argc, const char **argv)
{
    config_t cfg;
    int res = parse_config(&cfg, argc, argv);
    if (res != 0)
        return res;

    srand(clock());

    // Nested process code: initialize DSP
    dsp::context_t ctx;
    dsp::info_t *info;

    dsp::init();
    info = dsp::info();
    if (info != NULL)
    {
        printf("--------------------------------------------------------------------------------\n");
        printf("CPU information:\n");
        printf("  Architecture:   %s\n", info->arch);
        printf("  CPU string:     %s\n", info->cpu);
        printf("  CPU model:      %s\n", info->model);
        printf("  Features:       %s\n", info->features);
        free(info);
    }

    dsp::start(&ctx);

    // Launch tests
    switch (cfg.mode)
    {
        case PTEST:
            res = launch_ptest(&cfg);
            break;
        case UTEST:
            res = launch_utest(&cfg);
            break;
        default:
            break;
    }

    dsp::finish(&ctx);

    clear_config(&cfg);

    return res;
    /*srand(clock());
    lsp_trace("locale is: %s", setlocale(LC_CTYPE, NULL));
    return TEST::test(argc, argv);
    */
}
