/*************************************************************************
	> File Name: 123.c
	> Author: inspur
	> Mail: name@inspur.com 
	> Created Time: Wed 21 Oct 2015 11:28:44 AM CST
 ************************************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <math.h>
#include <sched.h>
#include <fcntl.h>


#include "rapl.h"
#include "cpuid.h"
#include "msr.h"

#define NUM_NODE 8
uint64_t      num_node = 0;

	// double power;
    double prev_sample[NUM_NODE][RAPL_NR_DOMAIN];
    double power_watt[NUM_NODE][RAPL_NR_DOMAIN];
    double cum_energy_J[NUM_NODE][RAPL_NR_DOMAIN];
    double cum_energy_mWh[NUM_NODE][RAPL_NR_DOMAIN];

    double start, end, interval_start;
    double total_elapsed_time;
    double interval_elapsed_time;

/* rapl msr availablility */
#define MSR_SUPPORT_MASK 0xff
unsigned char *msr_support_table;

/* Global Variables */
double RAPL_TIME_UNIT;
double RAPL_ENERGY_UNIT;
double RAPL_POWER_UNIT;

/* Pre-computed variables used for time-window calculation */
const double LN2 = 0.69314718055994530941723212145817656807550013436025;
const double A_F[4] = { 1.0, 1.1, 1.2, 1.3 };
const double A_LNF[4] = {
    0.0000000000000000000000000000000000000000000000000000000,
    0.0953101798043249348602046211453853175044059753417968750,
    0.1823215567939545922460098381634452380239963531494140625,
    0.2623642644674910595625760834082029759883880615234375000
};

typedef struct rapl_unit_multiplier_t {
    double power;
    double energy;
    double time;
} rapl_unit_multiplier_t;



uint64_t  num_nodes = 0;
uint64_t num_core_threads = 0; // number of physical threads per core
uint64_t num_pkg_threads = 0;  // number of physical threads per package
uint64_t num_pkg_cores = 0;    // number of cores per package
uint64_t os_cpu_count = 0;     // numbeer of OS cpus


APIC_ID_t *os_map;
APIC_ID_t **pkg_map;


// Parse the x2APIC_ID_t into SMT, core and package ID.
// http://software.intel.com/en-us/articles/intel-64-architecture-processor-topology-enumeration
void
parse_apic_id(cpuid_info_t info_l0, cpuid_info_t info_l1, APIC_ID_t *my_id){

    // Get the SMT ID
    uint64_t smt_mask_width = info_l0.eax & 0x1f;
    uint64_t smt_mask = ~((-1) << smt_mask_width);
    my_id->smt_id = info_l0.edx & smt_mask;

    // Get the core ID
    uint64_t core_mask_width = info_l1.eax & 0x1f;
    uint64_t core_mask = (~((-1) << core_mask_width ) ) ^ smt_mask;
    my_id->core_id = (info_l1.edx & core_mask) >> smt_mask_width;

    // Get the package ID
    uint64_t pkg_mask = (-1) << core_mask_width;
    my_id->pkg_id = (info_l1.edx & pkg_mask) >> core_mask_width;
}





// OS specific
int
bind_context(cpu_set_t *new_context, cpu_set_t *old_context) {

    int err = 0;
    int ret = 0;

    if (old_context != NULL) {
      err = sched_getaffinity(0, sizeof(cpu_set_t), old_context);
      if(0 != err)
        ret = MY_ERROR;
    }

    err += sched_setaffinity(0, sizeof(cpu_set_t), new_context);
    if(0 != err)
        ret = MY_ERROR;

    return ret;
}


int
bind_cpu(uint64_t cpu, cpu_set_t *old_context) {

    int err = 0;
    cpu_set_t cpu_context;

    CPU_ZERO(&cpu_context);
    CPU_SET(cpu, &cpu_context);
    err += bind_context(&cpu_context, old_context);

    return err;
}


// For documentation, see:
// http://software.intel.com/en-us/articles/intel-64-architecture-processor-topology-enumeration
int
build_topology() {

    int err;
    uint64_t i,j;
    uint64_t max_pkg = 0;
    os_cpu_count = sysconf(_SC_NPROCESSORS_CONF);
    cpu_set_t curr_context;
    cpu_set_t prev_context;

    // Construct an os map: os_map[APIC_ID ... APIC_ID]
    os_map = (APIC_ID_t *) malloc(os_cpu_count * sizeof(APIC_ID_t));

    for(i=0; i < os_cpu_count; i++){

        err = bind_cpu(i, &prev_context);

        cpuid_info_t info_l0 = get_processor_topology(0);
        cpuid_info_t info_l1 = get_processor_topology(1);

        os_map[i].os_id = i;
        parse_apic_id(info_l0, info_l1, &os_map[i]);

        num_core_threads = info_l0.ebx & 0xffff;
        num_pkg_threads = info_l1.ebx & 0xffff;

        if(os_map[i].pkg_id > max_pkg)
            max_pkg = os_map[i].pkg_id;

        err = bind_context(&prev_context, NULL);

        //printf("smt_id: %u core_id: %u pkg_id: %u os_id: %u\n",
        //   os_map[i].smt_id, os_map[i].core_id, os_map[i].pkg_id, os_map[i].os_id);

    }

    num_pkg_cores = num_pkg_threads / num_core_threads;
    num_nodes = max_pkg + 1;

    // Construct a pkg map: pkg_map[pkg id][APIC_ID ... APIC_ID]
    pkg_map = (APIC_ID_t **) malloc(num_nodes * sizeof(APIC_ID_t*));
    for(i = 0; i < num_nodes; i++)
        pkg_map[i] = (APIC_ID_t *) malloc(num_pkg_threads * sizeof(APIC_ID_t));

    uint64_t p, t;
    for(i = 0; i < os_cpu_count; i++){
        p = os_map[i].pkg_id;
        t = os_map[i].smt_id * num_pkg_cores + os_map[i].core_id;
        pkg_map[p][t] = os_map[i];
    }

    //for(i=0; i< num_nodes; i++)
    //    for(j=0; j<num_pkg_threads; j++)
    //        printf("smt_id: %u core_id: %u pkg_id: %u os_id: %u\n",
    //            pkg_map[i][j].smt_id, pkg_map[i][j].core_id,
    //            pkg_map[i][j].pkg_id, pkg_map[i][j].os_id);

    return err;
}


/*
 * read_msr
 *
 * Will return 0 on success and MY_ERROR on failure.
 */
int
read_msr(int       cpu,
         uint64_t  address,
         uint64_t *value)
{
    int   err = 0;
    char  msr_path[32];
    FILE *fp;

    sprintf(msr_path, "/dev/cpu/%d/msr", cpu);
    err = ((fp = fopen(msr_path, "r")) == NULL);
    if (!err)
        err = (fseek(fp, address, SEEK_CUR) != 0);
    if (!err)
        err = (fread(value, sizeof(uint64_t), 1, fp) != 1);
    if (fp != NULL)
        fclose(fp);
    return err;
}




/*!
 * \brief Check if MSR is supported on this machine.
 * \return 1 if supported, 0 otherwise
 */
uint64_t
is_supported_msr(uint64_t msr)
{
    return (uint64_t)msr_support_table[msr & MSR_SUPPORT_MASK];
}

/*!
 * \brief Check if power domain (PKG, PP0, PP1, DRAM) is supported on this machine.
 *
 * Currently server parts support: PKG, PP0 and DRAM and
 * client parts support PKG, PP0 and PP1.
 *
 * \return 1 if supported, 0 otherwise
 */
uint64_t
is_supported_domain(uint64_t power_domain)
{
    uint64_t supported = 0;

    switch (power_domain) {
    case RAPL_PKG:
        supported = is_supported_msr(MSR_RAPL_PKG_POWER_LIMIT);
        break;
    case RAPL_PP0:
        supported = is_supported_msr(MSR_RAPL_PP0_POWER_LIMIT);
        break;
    case RAPL_PP1:
        supported = is_supported_msr(MSR_RAPL_PP1_POWER_LIMIT);
        break;
    case RAPL_DRAM:
        supported = is_supported_msr(MSR_RAPL_DRAM_POWER_LIMIT);
        break;
    }

    return supported;
}


int
get_rapl_unit_multiplier(uint64_t                cpu,
                         rapl_unit_multiplier_t *unit_obj)
{
    int                        err = 0;
    uint64_t                   msr;
    rapl_unit_multiplier_msr_t unit_msr;

    err = !is_supported_msr(MSR_RAPL_POWER_UNIT);
    if (!err) {
        err = read_msr(cpu, MSR_RAPL_POWER_UNIT, &msr);
    }
    if (!err) {
        unit_msr = *(rapl_unit_multiplier_msr_t *)&msr;

        unit_obj->time = 1.0 / (double)(B2POW(unit_msr.time));
        unit_obj->energy = 1.0 / (double)(B2POW(unit_msr.energy));
        unit_obj->power = 1.0 / (double)(B2POW(unit_msr.power));
    }

    return err;
}






/* Utilities */

int
read_rapl_units()
{
    int                    err = 0;
    rapl_unit_multiplier_t unit_multiplier;

    err = get_rapl_unit_multiplier(0, &unit_multiplier);
    if (!err) {
        RAPL_TIME_UNIT = unit_multiplier.time;
        RAPL_ENERGY_UNIT = unit_multiplier.energy;
        RAPL_POWER_UNIT = unit_multiplier.power;
    }

    return err;
}





void
cpuid(uint32_t eax_in, uint32_t ecx_in,
      cpuid_info_t *ci)
{
    asm (
#if defined(__LP64__)           /* 64-bit architecture */
        "cpuid;"                /* execute the cpuid instruction */
        "movl %%ebx, %[ebx];"   /* save ebx output */
#else                           /* 32-bit architecture */
        "pushl %%ebx;"          /* save ebx */
        "cpuid;"                /* execute the cpuid instruction */
        "movl %%ebx, %[ebx];"   /* save ebx output */
        "popl %%ebx;"           /* restore ebx */
#endif
        : "=a"(ci->eax), [ebx] "=r"(ci->ebx), "=c"(ci->ecx), "=d"(ci->edx)
        : "a"(eax_in), "c"(ecx_in)
    );
}

uint32_t
get_processor_signature()
{
    cpuid_info_t info;
    cpuid(0x1, 0, &info);
    return info.eax;
}

cpuid_info_t
get_processor_topology(uint32_t level)
{
    cpuid_info_t info;
    cpuid(0xb, level, &info);
    return info;
}


/*!
 * \brief Get the number of RAPL nodes (package domain) on this machine.
 *
 * Get the number of package power domains, that you can control using RAPL.
 * This is equal to the number of CPU packages in the system.
 *
 * \return number of RAPL nodes.
 */
uint64_t
get_num_rapl_nodes_pkg()
{
    return num_nodes;
}

/*!
 * \brief Get the number of RAPL nodes (pp0 domain) on this machine.
 *
 * Get the number of pp0 (core) power domains, that you can control
 * using RAPL. Currently all the cores on a package belong to the same
 * power domain, so currently this is equal to the number of CPU packages in
 * the system.
 *
 * \return number of RAPL nodes.
 */
uint64_t
get_num_rapl_nodes_pp0()
{
    return num_nodes;
}

/*!
 * \brief Get the number of RAPL nodes (pp1 domain) on this machine.
 *
 * Get the number of pp1(uncore) power domains, that you can control using RAPL.
 * This is equal to the number of CPU packages in the system.
 *
 * \return number of RAPL nodes.
 */
uint64_t
get_num_rapl_nodes_pp1()
{
    return num_nodes;
}

/*!
 * \brief Get the number of RAPL nodes (DRAM domain) on this machine.
 *
 * Get the number of DRAM power domains, that you can control using RAPL.
 * This is equal to the number of CPU packages in the system.
 *
 * \return number of RAPL nodes.
 */
uint64_t
get_num_rapl_nodes_dram()
{
    return num_nodes;
}

uint64_t
pkg_node_to_cpu(uint64_t node)
{
    return pkg_map[node][0].os_id;
}

uint64_t
pp0_node_to_cpu(uint64_t node)
{
    return pkg_map[node][0].os_id;
}

uint64_t
pp1_node_to_cpu(uint64_t node)
{
    return pkg_map[node][0].os_id;
}

uint64_t
dram_node_to_cpu(uint64_t node)
{
    return pkg_map[node][0].os_id;
}

double
convert_to_watts(uint64_t raw)
{
    return RAPL_POWER_UNIT * raw;
}

double
convert_to_joules(uint64_t raw)
{
    return RAPL_ENERGY_UNIT * raw;
}

double
convert_to_seconds(uint64_t raw)
{
    return RAPL_TIME_UNIT * raw;
}

double
convert_from_limit_time_window(uint64_t Y,
                               uint64_t F)
{
    return B2POW(Y) * A_F[F] * RAPL_TIME_UNIT;
}

uint64_t
convert_from_watts(double converted)
{
    return converted / RAPL_POWER_UNIT;
}

uint64_t
compute_Y(uint64_t F,
          double   time)
{
    return (log((double)(time / RAPL_TIME_UNIT)) - A_LNF[F]) / LN2;
}


int
init_rapl()
{
    int      err = 0;
    uint32_t processor_signature;

    processor_signature = get_processor_signature();
    msr_support_table = (unsigned char*) calloc(MSR_SUPPORT_MASK, sizeof(unsigned char));

    /* RAPL MSRs by Table
     *   35-11: SandyBridge
     *     MSR_RAPL_POWER_UNIT  MSR_PKG_POWER_LIMIT   MSR_PKG_ENERGY_STATUS
     *     MSR_PKG_POWER_INFO   MSR_PP0_POWER_LIMIT   MSR_PP0_ENERGY_STATUS
     *     MSR_PP0_POLICY       MSR_PP0_PERF_STATUS
     *   35-12: SandyBridge Client
     *     MSR_PP1_POWER_LIMIT  MSR_PP1_ENERGY_STATUS MSR_PP1_POLICY
     *   35-13: SandyBridge Server
     *     MSR_PKG_PERF_STATUS  MSR_DRAM_POWER_LIMIT  MSR_DRAM_ENERGY_STATUS
     *     MSR_DRAM_PERF_STATUS MSR_DRAM_POWER_INFO
     *   35-14: IvyBridge
     *     N/A
     *   35-15: IvyBridge Server
     *     MSR_PKG_PERF_STATUS  MSR_DRAM_POWER_LIMIT  MSR_DRAM_ENERGY_STATUS
     *     MSR_DRAM_PERF_STATUS MSR_DRAM_POWER_INFO
     *   35-16: IvyBridge Server
     *     N/A
     *   35-17: Haswell
     *     N/A
     *   35-18: Haswell
     *     N/A
     *   35-19: Haswell
     *     MSR_PP1_POWER_LIMIT  MSR_PP1_ENERGY_STATUS MSR_PP1_POLICY
     */

    switch (processor_signature & 0xfffffff0) {
    case 0x306e0: /* IvyBridge server: 0x306eX (Tables 35:11,12,14,15,16) */
        msr_support_table[MSR_RAPL_POWER_UNIT & MSR_SUPPORT_MASK]          = 1;
        msr_support_table[MSR_RAPL_PKG_POWER_LIMIT & MSR_SUPPORT_MASK]     = 1;
        msr_support_table[MSR_RAPL_PKG_ENERGY_STATUS & MSR_SUPPORT_MASK]   = 1;
        msr_support_table[MSR_RAPL_PKG_PERF_STATUS & MSR_SUPPORT_MASK]     = 1;
        msr_support_table[MSR_RAPL_PKG_POWER_INFO & MSR_SUPPORT_MASK]      = 1;
        msr_support_table[MSR_RAPL_DRAM_POWER_LIMIT & MSR_SUPPORT_MASK]    = 1;
        msr_support_table[MSR_RAPL_DRAM_ENERGY_STATUS & MSR_SUPPORT_MASK]  = 1;
        msr_support_table[MSR_RAPL_DRAM_PERF_STATUS & MSR_SUPPORT_MASK]    = 1;
        msr_support_table[MSR_RAPL_DRAM_POWER_INFO & MSR_SUPPORT_MASK]     = 1;
        msr_support_table[MSR_RAPL_PP0_POWER_LIMIT & MSR_SUPPORT_MASK]     = 1;
        msr_support_table[MSR_RAPL_PP0_ENERGY_STATUS & MSR_SUPPORT_MASK]   = 1;
        msr_support_table[MSR_RAPL_PP0_POLICY & MSR_SUPPORT_MASK]          = 1;
        msr_support_table[MSR_RAPL_PP0_PERF_STATUS & MSR_SUPPORT_MASK]     = 1;
        msr_support_table[MSR_RAPL_PP1_POWER_LIMIT & MSR_SUPPORT_MASK]     = 1;
        msr_support_table[MSR_RAPL_PP1_ENERGY_STATUS & MSR_SUPPORT_MASK]   = 1;
        msr_support_table[MSR_RAPL_PP1_POLICY & MSR_SUPPORT_MASK]          = 1;
        break;
    case 0x40660: /* Haswell:            0x4066X (Tables 35:11,12,14,17,19) */
    case 0x40650: /* Haswell:            0x4065X (Tables 35:11,12,14,17,18,19) */
    case 0x306c0: /* Haswell:            0x306cX (Tables 35:11,12,14,17,19) */
    case 0x306a0: /* IvyBridge client:   0x306aX (Tables 35:11,12,14) */
    case 0x206a0: /* SandyBridge client: 0x206aX (Tables 35:11,12) */
        msr_support_table[MSR_RAPL_POWER_UNIT & MSR_SUPPORT_MASK]          = 1;
        msr_support_table[MSR_RAPL_PKG_POWER_LIMIT & MSR_SUPPORT_MASK]     = 1;
        msr_support_table[MSR_RAPL_PKG_ENERGY_STATUS & MSR_SUPPORT_MASK]   = 1;
        msr_support_table[MSR_RAPL_PKG_PERF_STATUS & MSR_SUPPORT_MASK]     = 0; //
        msr_support_table[MSR_RAPL_PKG_POWER_INFO & MSR_SUPPORT_MASK]      = 1;
        msr_support_table[MSR_RAPL_DRAM_POWER_LIMIT & MSR_SUPPORT_MASK]    = 0; //
        msr_support_table[MSR_RAPL_DRAM_ENERGY_STATUS & MSR_SUPPORT_MASK]  = 0; //
        msr_support_table[MSR_RAPL_DRAM_PERF_STATUS & MSR_SUPPORT_MASK]    = 0; //
        msr_support_table[MSR_RAPL_DRAM_POWER_INFO & MSR_SUPPORT_MASK]     = 0; //
        msr_support_table[MSR_RAPL_PP0_POWER_LIMIT & MSR_SUPPORT_MASK]     = 1;
        msr_support_table[MSR_RAPL_PP0_ENERGY_STATUS & MSR_SUPPORT_MASK]   = 1;
        msr_support_table[MSR_RAPL_PP0_POLICY & MSR_SUPPORT_MASK]          = 1;
        msr_support_table[MSR_RAPL_PP0_PERF_STATUS & MSR_SUPPORT_MASK]     = 1;
        msr_support_table[MSR_RAPL_PP1_POWER_LIMIT & MSR_SUPPORT_MASK]     = 1; //
        msr_support_table[MSR_RAPL_PP1_ENERGY_STATUS & MSR_SUPPORT_MASK]   = 1; //
        msr_support_table[MSR_RAPL_PP1_POLICY & MSR_SUPPORT_MASK]          = 1; //
        break;
    //case 0x20650: /* Valgrind */
    case 0x206d0: /* SandyBridge server: 0x206dX (Tables 35:11,13) */
        msr_support_table[MSR_RAPL_POWER_UNIT & MSR_SUPPORT_MASK]          = 1;
        msr_support_table[MSR_RAPL_PKG_POWER_LIMIT & MSR_SUPPORT_MASK]     = 1;
        msr_support_table[MSR_RAPL_PKG_ENERGY_STATUS & MSR_SUPPORT_MASK]   = 1;
        msr_support_table[MSR_RAPL_PKG_PERF_STATUS & MSR_SUPPORT_MASK]     = 1; //
        msr_support_table[MSR_RAPL_PKG_POWER_INFO & MSR_SUPPORT_MASK]      = 1;
        msr_support_table[MSR_RAPL_DRAM_POWER_LIMIT & MSR_SUPPORT_MASK]    = 1; //
        msr_support_table[MSR_RAPL_DRAM_ENERGY_STATUS & MSR_SUPPORT_MASK]  = 1; //
        msr_support_table[MSR_RAPL_DRAM_PERF_STATUS & MSR_SUPPORT_MASK]    = 1; //
        msr_support_table[MSR_RAPL_DRAM_POWER_INFO & MSR_SUPPORT_MASK]     = 1; //
        msr_support_table[MSR_RAPL_PP0_POWER_LIMIT & MSR_SUPPORT_MASK]     = 1;
        msr_support_table[MSR_RAPL_PP0_ENERGY_STATUS & MSR_SUPPORT_MASK]   = 1;
        msr_support_table[MSR_RAPL_PP0_POLICY & MSR_SUPPORT_MASK]          = 1;
        msr_support_table[MSR_RAPL_PP0_PERF_STATUS & MSR_SUPPORT_MASK]     = 1;
        msr_support_table[MSR_RAPL_PP1_POWER_LIMIT & MSR_SUPPORT_MASK]     = 0; //
        msr_support_table[MSR_RAPL_PP1_ENERGY_STATUS & MSR_SUPPORT_MASK]   = 0; //
        msr_support_table[MSR_RAPL_PP1_POLICY & MSR_SUPPORT_MASK]          = 0; //
        break;
    default:
        fprintf(stderr, "RAPL not supported, or machine model %x not recognized.\n", processor_signature);
        return MY_ERROR;
    }

    err = read_rapl_units();
    err += build_topology();

    /* 32 is the width of these fields when they are stored */
    MAX_ENERGY_STATUS_JOULES = (double)(RAPL_ENERGY_UNIT * (pow(2, 32) - 1));
    MAX_THROTTLED_TIME_SECONDS = (double)(RAPL_TIME_UNIT * (pow(2, 32) - 1));

    return err;
}



/*!
 * \brief Terminate the power_gov library.
 *
 * Call this function function to cleanup resources and terminate the
 * power_gov library.
 * \return 0 on success
 */
int
terminate_rapl()
{
    uint64_t i;

    if(NULL != os_map)
        free(os_map);

    if(NULL != pkg_map){
        for(i = 0; i < num_nodes; i++)
            free(pkg_map[i]);
        free(pkg_map);
    }

    if(NULL != msr_support_table)
        free(msr_support_table);

    return 0;
}


// TODO: Improve the error handling of this function
// According to documentation the RDTSC can fail
// under protected or virtual mode, when certain flags are set
int
read_tsc(uint64_t *tsc)
{
    uint32_t d;
    uint32_t a;

    asm("rdtsc;"
        "mov %%edx, %0;"      // move edx into d
        "mov %%eax, %1;"      // move eax into a
        : "=r" (d), "=r" (a)  // output
        :                     // input
        : "%edx", "eax"       // clobbered regiters
        );

    *tsc = (uint64_t)d << 32 | a;

    return 0;
}



/* Required by Power Gadget */

int
get_os_freq(uint64_t cpu, uint64_t *freq)
{
    char path[60];
    int ret = 0;
    int out = 0;
    FILE *fp;

    out = sprintf(path, "%s%u%s", "/sys/devices/system/cpu/cpu",cpu,"/cpufreq/cpuinfo_cur_freq");

    if(out > 0)
        fp = fopen(path, "r");

    if(NULL != fp){
        fscanf(fp, "%u", freq);
        fclose(fp);
    }
    else{
        ret = MY_ERROR;
    }

    return ret;
}


int
get_total_energy_consumed(uint64_t  cpu,
                          uint64_t  msr_address,
                          double   *total_energy_consumed_joules)
{
    int                 err = 0;
    uint64_t            msr;
    energy_status_msr_t domain_msr;
    cpu_set_t old_context;

    err = !is_supported_msr(msr_address);
    if (!err) {
        bind_cpu(cpu, &old_context); // improve performance on Linux
        err = read_msr(cpu, msr_address, &msr);
        bind_context(&old_context, NULL);
    }

    if(!err) {
        domain_msr = *(energy_status_msr_t *)&msr;

        *total_energy_consumed_joules = convert_to_joules(domain_msr.total_energy_consumed);
    }

    return err;
}



// Uses the OS (and not the PMU counters) to retrieve the frequency
int
get_pp0_freq_mhz(uint64_t node, uint64_t *freq)
{
    int ret = 0;
    int i;

    // If all the cores are on the same power domain, report the average freq
    if( get_num_rapl_nodes_pp0() == get_num_rapl_nodes_pkg())
    {
        uint64_t sum_freq = 0;
        uint64_t cpu_freq = 0;

        for(i=0; i<num_pkg_threads; i++)
        {
            uint64_t os_cpu = pkg_map[node][i].os_id;
            ret = get_os_freq(os_cpu, &cpu_freq);
            sum_freq += cpu_freq;
        }

        if(0 == ret)
            *freq =  (sum_freq / num_pkg_threads) / 1000.0;
    }
    else
    {
        uint64_t cpu_freq = 0;
        uint64_t os_cpu = pp0_node_to_cpu(node);
        ret = get_os_freq(os_cpu, &cpu_freq);
        if(0 == ret)
            *freq = cpu_freq / 1000.0;
    }

    return ret;
}

/*!
 * \brief Get a pointer to the RAPL PKG energy consumed register.
 *
 * This read-only register provides energy consumed in joules
 * for the package power domain since the last machine reboot (or energy register wraparound)
 *
 * \return 0 on success, -1 otherwise
 */
int
get_pkg_total_energy_consumed(uint64_t  node,
                              double   *total_energy_consumed_joules)
{
    uint64_t cpu = pkg_node_to_cpu(node);
    return get_total_energy_consumed(cpu, MSR_RAPL_PKG_ENERGY_STATUS, total_energy_consumed_joules);
}

/*!
 * \brief Get a pointer to the RAPL PP0 energy consumed register.
 *
 * This read-only register provides energy consumed in joules
 * for the PP0 (core) power domain since the last machine reboot (or energy register wraparound)
 *
 * \return 0 on success, -1 otherwise
 */
int
get_pp0_total_energy_consumed(uint64_t  node,
                              double   *total_energy_consumed_joules)
{
    uint64_t cpu = pp0_node_to_cpu(node);
    return get_total_energy_consumed(cpu, MSR_RAPL_PP0_ENERGY_STATUS, total_energy_consumed_joules);
}



/*!
 * \brief Get a pointer to the RAPL PP1 energy consumed register.
 *
 * (Client parts only)
 *
 * This read-only register provides energy consumed in joules
 * for the PP1 (uncore) power domain since the last machine reboot (or energy register wraparound)
 *
 * \return 0 on success, -1 otherwise
 */
int
get_pp1_total_energy_consumed(uint64_t  node,
                              double   *total_energy_consumed_joules)
{
    uint64_t cpu = pp1_node_to_cpu(node);
    return get_total_energy_consumed(cpu, MSR_RAPL_PP1_ENERGY_STATUS, total_energy_consumed_joules);
}



/*!
 * \brief Get a pointer to the RAPL DRAM energy consumed register.
 *
 * (Server parts only)
 *
 * This read-only register provides energy consumed in joules
 * for the DRAM power domain since the last machine reboot (or energy register wraparound)
 *
 * \return 0 on success, -1 otherwise
 */
int
get_dram_total_energy_consumed(uint64_t  node,
                               double   *total_energy_consumed_joules)
{
    uint64_t cpu = dram_node_to_cpu(node);
    return get_total_energy_consumed(cpu, MSR_RAPL_DRAM_ENERGY_STATUS, total_energy_consumed_joules);
}



double
get_rapl_energy_info(uint64_t power_domain, uint64_t node)
{
    int          err;
    double       total_energy_consumed;

    switch (power_domain) {
    case PKG:
        err = get_pkg_total_energy_consumed(node, &total_energy_consumed);
        break;
    case PP0:
        err = get_pp0_total_energy_consumed(node, &total_energy_consumed);
        break;
    case PP1:
        err = get_pp1_total_energy_consumed(node, &total_energy_consumed);
        break;
    case DRAM:
        err = get_dram_total_energy_consumed(node, &total_energy_consumed);
        break;
    default:
        err = MY_ERROR;
        break;
    }

    return total_energy_consumed;
}



void
convert_time_to_string(struct timeval tv, char* time_buf)
{
    time_t sec;
    int msec;
    struct tm *timeinfo;
    char tmp_buf[9];

    sec = tv.tv_sec;
    timeinfo = localtime(&sec);
    msec = tv.tv_usec/1000;

    strftime(tmp_buf, 9, "%H:%M:%S", timeinfo);
    sprintf(time_buf, "%s:%d",tmp_buf,msec);
}



double
convert_time_to_sec(struct timeval tv)
{
    double elapsed_time = (double)(tv.tv_sec) + ((double)(tv.tv_usec)/1000000);
    return elapsed_time;
}



void
do_print_energy_info()
{
    int i = 0;
    int domain = 0;
    uint64_t node = 0;
    double new_sample;
    double delta;
    double power;



    char time_buffer[32];
    struct timeval tv;
    int msec;
    uint64_t tsc;
    uint64_t freq;

    /* don't buffer if piped */
//   setbuf(stdout, NULL);

#if 0

    /* Print header */
    fprintf(stdout, "System Time,RDTSC,Elapsed Time (sec),");
    for (i = node; i < num_node; i++) {
        fprintf(stdout, "IA Frequency_%d (MHz),",i);
        if(is_supported_domain(RAPL_PKG))
            fprintf(stdout,"Processor Power_%d (Watt),Cumulative Processor Energy_%d (Joules),Cumulative Processor Energy_%d (mWh),", i,i,i);
        if(is_supported_domain(RAPL_PP0))
            fprintf(stdout, "IA Power_%d (Watt),Cumulative IA Energy_%d (Joules),Cumulative IA Energy_%d(mWh),", i,i,i);
        if(is_supported_domain(RAPL_PP1))
            fprintf(stdout, "GT Power_%d (Watt),Cumulative GT Energy_%d (Joules),Cumulative GT Energy_%d(mWh)", i,i,i);
        if(is_supported_domain(RAPL_DRAM))
            fprintf(stdout, "DRAM Power_%d (Watt),Cumulative DRAM Energy_%d (Joules),Cumulative DRAM Energy_%d(mWh),", i,i,i);
    }
    fprintf(stdout, "\n");
#endif
    /* Read initial values */
#if 1
    for (i = node; i < num_node; i++) {
        for (domain = 0; domain < RAPL_NR_DOMAIN; ++domain) {
            if(is_supported_domain(domain)) {
                prev_sample[i][domain] = get_rapl_energy_info(domain, i);
            }
        }
    }

    gettimeofday(&tv, NULL);
    start = convert_time_to_sec(tv);
    end = start;
#endif

    /* Begin sampling */
    while (1) 
	{

      usleep(2000000);

        gettimeofday(&tv, NULL);
        interval_start = convert_time_to_sec(tv);
        interval_elapsed_time = interval_start - end;

        for (i = node; i < num_node; i++) {
            for (domain = 0; domain < RAPL_NR_DOMAIN; ++domain) {
                if(is_supported_domain(domain)) {
                    new_sample = get_rapl_energy_info(domain, i);
                    delta = new_sample - prev_sample[i][domain];

                    /* Handle wraparound */
                    if (delta < 0) {
                        delta += MAX_ENERGY_STATUS_JOULES;
                    }

                    prev_sample[i][domain] = new_sample;

                    // Use the computed elapsed time between samples (and not
                    // just the sleep delay, in order to more accourately account for
                    // the delay between samples
                    power_watt[i][domain] = delta / interval_elapsed_time;
                    cum_energy_J[i][domain] += delta;
                    cum_energy_mWh[i][domain] = cum_energy_J[i][domain] / 3.6; // mWh
                }
            }
        }

        gettimeofday(&tv, NULL);
        end = convert_time_to_sec(tv);
        total_elapsed_time = end - start;
        convert_time_to_string(tv, time_buffer);

        read_tsc(&tsc);
//        fprintf(stdout,"%s,%llu,%.4lf,", time_buffer, tsc, total_elapsed_time);
        for (i = node; i < num_node; i++) {
//            get_pp0_freq_mhz(i, &freq);
//            fprintf(stdout, "%u,", freq);
            for (domain = 0; domain < RAPL_NR_DOMAIN; ++domain) {
                if(is_supported_domain(domain)) {
                    fprintf(stdout, "%.4lf,%.4lf,%.4lf,",
                            power_watt[i][domain], cum_energy_J[i][domain], cum_energy_mWh[i][domain]);
                }
            
			 }
        fprintf(stdout, "\n");

        // check to see if we are done
       // if(total_elapsed_time >= duration)
       //     break;
		 
		}
	}


#if 0
    end = clock();
    /* Print summary */
    fprintf(stdout, "\nTotal Elapsed Time(sec)=%.4lf\n\n", total_elapsed_time);
    for (i = node; i < num_node; i++) {
        if(is_supported_domain(RAPL_PKG)){
        //    fprintf(stdout, "Total Processor Energy_%d(Joules)=%.4lf\n", i, cum_energy_J[i][RAPL_PKG]);
        //    fprintf(stdout, "Total Processor Energy_%d(mWh)=%.4lf\n", i, cum_energy_mWh[i][RAPL_PKG]);
            fprintf(stdout, "Average Processor Power_%d(Watt)=%.4lf\n\n", i, cum_energy_J[i][RAPL_PKG]/total_elapsed_time);
        }
        if(is_supported_domain(RAPL_PP0)){
        //    fprintf(stdout, "Total IA Energy_%d(Joules)=%.4lf\n", i, cum_energy_J[i][RAPL_PP0]);
        //    fprintf(stdout, "Total IA Energy_%d(mWh)=%.4lf\n", i, cum_energy_mWh[i][RAPL_PP0]);
            fprintf(stdout, "Average IA Power_%d(Watt)=%.4lf\n\n", i, cum_energy_J[i][RAPL_PP0]/total_elapsed_time);
        }
        if(is_supported_domain(RAPL_PP1)){
        //    fprintf(stdout, "Total GT Energy_%d(Joules)=%.4lf\n", i, cum_energy_J[i][RAPL_PP1]);
        //    fprintf(stdout, "Total GT Energy_%d(mWh)=%.4lf\n", i, cum_energy_mWh[i][RAPL_PP1]);
            fprintf(stdout, "Average GT Power_%d(Watt)=%.4lf\n\n", i, cum_energy_J[i][RAPL_PP1]/total_elapsed_time);
        }
        if(is_supported_domain(RAPL_DRAM)){
        //    fprintf(stdout, "Total DRAM Energy_%d(Joules)=%.4lf\n", i, cum_energy_J[i][RAPL_DRAM]);
        //    fprintf(stdout, "Total DRAM Energy_%d(mWh)=%.4lf\n", i, cum_energy_mWh[i][RAPL_DRAM]);
            fprintf(stdout, "Average DRAM Power_%d(Watt)=%.4lf\n\n", i, cum_energy_J[i][RAPL_DRAM]/total_elapsed_time);
        }
    }
    read_tsc(&tsc);
	terminate_rapl();/////////////////////////////////////////////////////////////////////////
    fprintf(stdout,"TSC=%llu\n", tsc);

#endif
	
}



int
main(int argc, char **argv)
{
    int i = 0;
    int ret = 0;

    struct timeval tv;

    // First init the RAPL library
    if (0 != init_rapl()) {
        fprintf(stdout, "Init failed!\n");
	terminate_rapl();
        return MY_ERROR;
    }
    num_node = get_num_rapl_nodes_pkg();


    gettimeofday(&tv, NULL);
    start = convert_time_to_sec(tv);
    end = start;

    do_print_energy_info();

    terminate_rapl();
}
