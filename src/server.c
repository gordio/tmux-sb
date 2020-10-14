#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h> // with sys/socket.h for cross
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>

#if defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/param.h>
    #if defined(BSD)
        #include <sys/sysctl.h>
    #endif
    #if defined(__APPLE__)
        #include <sys/types.h>
        #include <sys/sysctl.h>
    #endif
    #if defined(__MACH__)
        #include <mach/mach.h>
    #endif
#else
    #error "Unable to define getMemorySize( ) for an unknown OS."
#endif

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <err.h>

#include "main.h"
#include "server.h"
#include "metrics.h"


#ifndef MIN
    #define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
    #define MAX(a,b) ((a)>(b)?(a):(b))
#endif


static int init_sock(struct sockaddr_un *addr);
static void deinit_sock(int sock, struct sockaddr_un *addr);
inline static FILE * open_file(char *name);
static void signal_handler(int signal);


static int sock;
static struct sockaddr_un addr;

void
start_server(const char *file)
{
    int csock;
    char *buf;
    socklen_t addr_len;

    // mem
    FILE *ram_fd = NULL;
    unsigned int memTotal, memFree, memAvailable, memBuffed, memCached, memPrc;
    // swap
#ifdef __APPLE__
    int mib[CTL_MAXNAME], state;
    size_t size;
    struct xsw_usage xsu;
#else
    struct sysinfo sysInfo;
#endif
    unsigned long swpFree, swpTotal, swpPrc;
    // cpu
    FILE *cpu_fd = NULL;
    unsigned long int a[9], b[9], work_over_period, total_over_period;
    int cpu_prc = 0;


    if (!(buf = malloc(BUF_SIZE))) {
        err(1, "Can't allocate buffer %i KiB.", BUF_SIZE/1024);
    }

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, file, (sizeof addr.sun_path) - 1);

    // Create server socket
    sock = init_sock(&addr);

    // Connect signals
    signal(SIGHUP, signal_handler);
    signal(SIGINT, signal_handler);  // 2 Terminal interrupt signal. (receive ^C)
    signal(SIGABRT, signal_handler); // 6 Process abort signal.
    signal(SIGSEGV, signal_handler); // Invalid memory reference.
    signal(SIGTERM, signal_handler); // 15 Termination signal.
    signal(SIGFPE, signal_handler);  // Erroneous arithmetic operation.
    signal(SIGILL, signal_handler);  // Illegal instruction.
    signal(SIGQUIT, signal_handler); // Termination signal.

    // Open files
    #ifdef __APPLE__
    #else
    ram_fd = open_file("/proc/meminfo");
    #endif
    cpu_fd = open_file("/proc/stat");

    // Fix empty
    if (7 != fscanf(cpu_fd, "cpu  %lu %lu %lu %lu %lu %lu %lu", &b[0], &b[1], &b[2], &b[3], &b[4], &b[5], &b[6])) {
        errx(1, "Fail parse cpu stat.");
    } else {
        // work (user + nice + system)
        b[7] = b[0] + b[1] + b[2];
        // total
        b[8] = b[7] + b[3] + b[4] + b[5] + b[6];
    }
    rewind(cpu_fd);
    setbuf(cpu_fd, NULL);


    while (true) {
        addr_len = sizeof addr;

        // Wait connecting...
        if (-1 == (csock = accept(sock, (struct sockaddr *) &addr, &addr_len))) {
            errx(1, "Accept error %i: %s", errno, strerror(errno));
        }

        // cpu  nice user system irq soft_irq io_wait steal guest
        if (7 != fscanf(cpu_fd, "cpu  %lu %lu %lu %lu %lu %lu %lu", &a[0], &a[1], &a[2], &a[3], &a[4], &a[5], &a[6])) {
            errx(1, "Fail parse cpu stat.");
        }
        // Work (user + nice + system)
        a[7] = a[0] + a[1] + a[2];
        // Total
        a[8] = a[7] + a[3] + a[4] + a[5] + a[6];
        // Periods
        work_over_period = a[7] - b[7];
        total_over_period = a[8] - b[8];

        // Calc CPU load
        cpu_prc = ((float)work_over_period / total_over_period) * 100.0;

        // Min = 0, max = 100
        cpu_prc = MIN(100, MAX(0, cpu_prc));

        // a -> b
        memcpy(b, a, sizeof b);

        // Reset fd
        rewind(cpu_fd);
        setbuf(cpu_fd, NULL);

#ifdef __APPLE__
        mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
        vm_statistics_data_t vmstat;
        if (KERN_SUCCESS != host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&vmstat, &count)) {
            // FIXME: Error
        }

        memTotal = vmstat.wire_count + vmstat.active_count + vmstat.inactive_count + vmstat.free_count;
        //double wired = vmstat.wire_count / memTotal;
        //double active = vmstat.active_count / memTotal;
        //double inactive = vmstat.inactive_count / memTotal;
        memPrc = vmstat.free_count / memTotal;
#else
        // Memory
        if (1 != fscanf(ram_fd, "MemTotal: %u kB\n", &memTotal) ||
            1 != fscanf(ram_fd, "MemFree: %u kB\n", &memFree) ||
            1 != fscanf(ram_fd, "MemAvailable: %u kB\n", &memAvailable) ||
            1 != fscanf(ram_fd, "Buffers: %u kB\n", &memBuffed) ||
            1 != fscanf(ram_fd, "Cached: %u kB\n", &memCached)) {
            errx(-1, "Fail parse mem file.");
        }
        memPrc = (memFree + memCached) / (memTotal / 100);

        rewind(ram_fd);
        setbuf(ram_fd, NULL);
#endif

        // Swap
#ifdef __APPLE__
        if (sysctl(mib, 2, &xsu, &size, NULL, 0) != 0) {
            exit(-1);
        }
        swpTotal = ((double) xsu.xsu_total) / (1024.0 * 1024.0);
        // swpUsed = ((double) xsu.xsu_used) / (1024.0 * 1024.0);
        swpFree = ((double) xsu.xsu_avail) / (1024.0 * 1024.0);
        swpPrc = swpFree / (swpTotal / 100);
#else
        sysinfo(&sysInfo);
        swpTotal = sysInfo.totalswap * sysInfo.mem_unit;
        swpFree = sysInfo.freeswap * sysInfo.mem_unit;
        swpPrc = swpFree / (swpTotal / 100);
#endif

        // if swap exist (exclude divide by zero)
        if (0 != swpTotal) {
            snprintf(buf, BUF_SIZE - 1,
                    "CPU:%3.1u%% #[fg=green]|#[fg=default] MEM:%3.1i%% SWAP:%2.1lu%%",
                    cpu_prc, 100 - memPrc, 100 - swpPrc);
        } else {
            snprintf(buf, BUF_SIZE - 1,
                    "CPU:%3.1u%% #[fg=green]|#[fg=default] MEM:%3.1i%%",
                    cpu_prc, 100 - memPrc);
        }

        buf[BUF_SIZE - 1] = '\0'; // hard deny overflow

        send(csock, buf, strlen(buf), 0);

        // Close client socket
        close(csock);
    }

    // Close server socket
    deinit_sock(sock, &addr);

    // Close files
    fclose(cpu_fd);
    fclose(ram_fd);
}


static int
init_sock(struct sockaddr_un *addr)
{
    int sock;


    // Create socket
    if (-1 == (sock = socket(AF_UNIX, SOCK_STREAM, 0))) {
        errx(1, "Can't create socket.");
    }

    // Bind to file
    if (-1 == bind(sock, (struct sockaddr *) addr, sizeof *addr)) {
        errx(1, "Can't bind socket to file '%s': %s", addr->sun_path, strerror(errno));
    }

    // Listening
    if (-1 == listen(sock, LISTEN_BACKLOG)) {
        errx(1, "Can't list socket: %s", strerror(errno));
    }

    return sock;
}


static void
deinit_sock(int sock, struct sockaddr_un *addr)
{
    close(sock);
    unlink(addr->sun_path);
}


inline static FILE *
open_file(char *name)
{
    FILE *fd = NULL;


    if (!(fd = fopen(name, "r"))) {
        errx(1, "Failed to open file '%s': %s", name, strerror(errno));
    }

    return fd;
}


static void
signal_handler(int signal)
{
    switch (signal) {
        case SIGHUP:
            // TODO: Reread config.
            break;

        default:
            deinit_sock(sock, &addr);
            break;
    }
}
