/*
 *  linux/kernel/cpufreq.c
 *
 *  Copyright (C) 2001 Russell King
 *            (C) 2002 - 2003 Dominik Brodowski <linux@brodo.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/notifier.h>
#include <linux/cpufreq.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/slab.h>

#include <asm/semaphore.h>
/**
 * The "cpufreq driver" - the arch- or hardware-dependend low
 * level driver of CPUFreq support, and its spinlock. This lock
 * also protects the cpufreq_cpu_data array.
 */
static struct cpufreq_driver   	*cpufreq_driver;
static struct cpufreq_policy	*cpufreq_cpu_data[NR_CPUS];
static spinlock_t		cpufreq_driver_lock = SPIN_LOCK_UNLOCKED;

/* internal prototype */
static int __cpufreq_governor(struct cpufreq_policy *policy, unsigned int event);


/**
 * Two notifier lists: the "policy" list is involved in the 
 * validation process for a new CPU frequency policy; the 
 * "transition" list for kernel code that needs to handle
 * changes to devices when the CPU clock speed changes.
 * The mutex locks both lists.
 */
static struct notifier_block    *cpufreq_policy_notifier_list;
static struct notifier_block    *cpufreq_transition_notifier_list;
static DECLARE_RWSEM		(cpufreq_notifier_rwsem);


static LIST_HEAD(cpufreq_governor_list);
static DECLARE_MUTEX		(cpufreq_governor_sem);

/*
 * backport info:
 * we don't have a kobj we can use for ref-counting, so use a
 * "unsigned int policy->use_count" and an "unload_sem" [idea from
 * Pat Mochel's struct driver unload_sem] for proper reference counting.
 */

static struct cpufreq_policy * cpufreq_cpu_get(unsigned int cpu)
{
	struct cpufreq_policy *data;
	unsigned long flags;

	if (cpu >= NR_CPUS)
		goto err_out;

	/* get the cpufreq driver */
	spin_lock_irqsave(&cpufreq_driver_lock, flags);

	if (!cpufreq_driver)
		goto err_out_unlock;

	/* get the CPU */
	data = cpufreq_cpu_data[cpu];

	if (!data)
		goto err_out_unlock;

	if (!data->use_count)
		goto err_out_unlock;

	data->use_count += 1;

	spin_unlock_irqrestore(&cpufreq_driver_lock, flags);

	return data;

 err_out_unlock:
	spin_unlock_irqrestore(&cpufreq_driver_lock, flags);
 err_out:
	return NULL;
}

static void cpufreq_cpu_put(struct cpufreq_policy *data)
{
	unsigned long flags;

	spin_lock_irqsave(&cpufreq_driver_lock, flags);
	data->use_count -= 1;
	if (!data->use_count) {
		spin_unlock_irqrestore(&cpufreq_driver_lock, flags);
		up(&data->unload_sem);
		return;
	}
	spin_unlock_irqrestore(&cpufreq_driver_lock, flags);
}

/*********************************************************************
 *                          SYSFS INTERFACE                          *
 *********************************************************************/

/**
 * cpufreq_parse_governor - parse a governor string
 */
int cpufreq_parse_governor (char *str_governor, unsigned int *policy,
				struct cpufreq_governor **governor)
{
	if (!strnicmp(str_governor, "performance", CPUFREQ_NAME_LEN)) {
		*policy = CPUFREQ_POLICY_PERFORMANCE;
		return 0;
	} else if (!strnicmp(str_governor, "powersave", CPUFREQ_NAME_LEN)) {
		*policy = CPUFREQ_POLICY_POWERSAVE;
		return 0;
	} else 	{
		struct cpufreq_governor *t;
		down(&cpufreq_governor_sem);
		if (!cpufreq_driver || !cpufreq_driver->target)
			goto out;
		list_for_each_entry(t, &cpufreq_governor_list, governor_list) {
			if (!strnicmp(str_governor,t->name,CPUFREQ_NAME_LEN)) {
				*governor = t;
				*policy = CPUFREQ_POLICY_GOVERNOR;
				up(&cpufreq_governor_sem);
				return 0;
			}
		}
	out:
		up(&cpufreq_governor_sem);
	}
	return -EINVAL;
}
EXPORT_SYMBOL_GPL(cpufreq_parse_governor);


/* backport info:
 * all the sysfs stuff is missing -- of course
 */

/**
 * cpufreq_add_dev - add a CPU device
 *
 * Adds the cpufreq interface for a CPU device. 
 */
static int cpufreq_add_dev (unsigned int cpu)
{
	int ret = 0;
	struct cpufreq_policy new_policy;
	struct cpufreq_policy *policy;
	unsigned long flags;

	policy = kmalloc(sizeof(struct cpufreq_policy), GFP_KERNEL);
	if (!policy)
		return -ENOMEM;
	memset(policy, 0, sizeof(struct cpufreq_policy));

	policy->cpu = cpu;
	policy->use_count = 1;
	init_MUTEX_LOCKED(&policy->lock);
	init_MUTEX_LOCKED(&policy->unload_sem);

	/* call driver. From then on the cpufreq must be able
	 * to accept all calls to ->verify and ->setpolicy for this CPU
	 */
	ret = cpufreq_driver->init(policy);
	if (ret)
		goto err_out;

	memcpy(&new_policy, policy, sizeof(struct cpufreq_policy));

	spin_lock_irqsave(&cpufreq_driver_lock, flags);
	cpufreq_cpu_data[cpu] = policy;
	spin_unlock_irqrestore(&cpufreq_driver_lock, flags);

	up(&policy->lock);
	
	/* set default policy */
	ret = cpufreq_set_policy(&new_policy);
	if (ret)
		goto err_out_unregister;

	return 0;


 err_out_unregister:
	spin_lock_irqsave(&cpufreq_driver_lock, flags);
	cpufreq_cpu_data[cpu] = NULL;
	spin_unlock_irqrestore(&cpufreq_driver_lock, flags);

 err_out:
	kfree(policy);
	return ret;
}


/**
 * cpufreq_remove_dev - remove a CPU device
 *
 * Removes the cpufreq interface for a CPU device.
 */
static int cpufreq_remove_dev (unsigned int cpu)
{
	unsigned long flags;
	struct cpufreq_policy *data;

	spin_lock_irqsave(&cpufreq_driver_lock, flags);
	data = cpufreq_cpu_data[cpu];
	if (!data) {
		spin_unlock_irqrestore(&cpufreq_driver_lock, flags);
		return -EINVAL;
	}
	cpufreq_cpu_data[cpu] = NULL;

	data->use_count -= 1;
	if (!data->use_count) {
		spin_unlock_irqrestore(&cpufreq_driver_lock, flags);
		up(&data->unload_sem);
	} else {
		spin_unlock_irqrestore(&cpufreq_driver_lock, flags);
		/* this will sleep until data->use_count gets to zero */
		down(&data->unload_sem);
		up(&data->unload_sem);
	}

	if (cpufreq_driver->target)
		__cpufreq_governor(data, CPUFREQ_GOV_STOP);

	if (cpufreq_driver->exit)
		cpufreq_driver->exit(data);

	kfree(data);

	return 0;
}


/*********************************************************************
 *                     NOTIFIER LISTS INTERFACE                      *
 *********************************************************************/

/**
 *	cpufreq_register_notifier - register a driver with cpufreq
 *	@nb: notifier function to register
 *      @list: CPUFREQ_TRANSITION_NOTIFIER or CPUFREQ_POLICY_NOTIFIER
 *
 *	Add a driver to one of two lists: either a list of drivers that 
 *      are notified about clock rate changes (once before and once after
 *      the transition), or a list of drivers that are notified about
 *      changes in cpufreq policy.
 *
 *	This function may sleep, and has the same return conditions as
 *	notifier_chain_register.
 */
int cpufreq_register_notifier(struct notifier_block *nb, unsigned int list)
{
	int ret;

	down_write(&cpufreq_notifier_rwsem);
	switch (list) {
	case CPUFREQ_TRANSITION_NOTIFIER:
		ret = notifier_chain_register(&cpufreq_transition_notifier_list, nb);
		break;
	case CPUFREQ_POLICY_NOTIFIER:
		ret = notifier_chain_register(&cpufreq_policy_notifier_list, nb);
		break;
	default:
		ret = -EINVAL;
	}
	up_write(&cpufreq_notifier_rwsem);

	return ret;
}
EXPORT_SYMBOL(cpufreq_register_notifier);


/**
 *	cpufreq_unregister_notifier - unregister a driver with cpufreq
 *	@nb: notifier block to be unregistered
 *      @list: CPUFREQ_TRANSITION_NOTIFIER or CPUFREQ_POLICY_NOTIFIER
 *
 *	Remove a driver from the CPU frequency notifier list.
 *
 *	This function may sleep, and has the same return conditions as
 *	notifier_chain_unregister.
 */
int cpufreq_unregister_notifier(struct notifier_block *nb, unsigned int list)
{
	int ret;

	down_write(&cpufreq_notifier_rwsem);
	switch (list) {
	case CPUFREQ_TRANSITION_NOTIFIER:
		ret = notifier_chain_unregister(&cpufreq_transition_notifier_list, nb);
		break;
	case CPUFREQ_POLICY_NOTIFIER:
		ret = notifier_chain_unregister(&cpufreq_policy_notifier_list, nb);
		break;
	default:
		ret = -EINVAL;
	}
	up_write(&cpufreq_notifier_rwsem);

	return ret;
}
EXPORT_SYMBOL(cpufreq_unregister_notifier);


/*********************************************************************
 *                              GOVERNORS                            *
 *********************************************************************/


int __cpufreq_driver_target(struct cpufreq_policy *policy,
			    unsigned int target_freq,
			    unsigned int relation)
{
	return cpufreq_driver->target(policy, target_freq, relation);
}
EXPORT_SYMBOL_GPL(__cpufreq_driver_target);


int cpufreq_driver_target(struct cpufreq_policy *policy,
			  unsigned int target_freq,
			  unsigned int relation)
{
	unsigned int ret;

	policy = cpufreq_cpu_get(policy->cpu);
	if (!policy)
		return -EINVAL;

	down(&policy->lock);

	ret = __cpufreq_driver_target(policy, target_freq, relation);

	up(&policy->lock);

	cpufreq_cpu_put(policy);

	return ret;
}
EXPORT_SYMBOL_GPL(cpufreq_driver_target);


static int __cpufreq_governor(struct cpufreq_policy *policy, unsigned int event)
{
	int ret = 0;

	switch (policy->policy) {
	case CPUFREQ_POLICY_POWERSAVE: 
		if ((event == CPUFREQ_GOV_LIMITS) || (event == CPUFREQ_GOV_START)) {
			ret = __cpufreq_driver_target(policy, policy->min, CPUFREQ_RELATION_L);
		}
		break;
	case CPUFREQ_POLICY_PERFORMANCE:
		if ((event == CPUFREQ_GOV_LIMITS) || (event == CPUFREQ_GOV_START)) {
			ret = __cpufreq_driver_target(policy, policy->max, CPUFREQ_RELATION_H);
		}
		break;
	case CPUFREQ_POLICY_GOVERNOR:
		ret = policy->governor->governor(policy, event);
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}


int cpufreq_governor(unsigned int cpu, unsigned int event)
{
	int ret = 0;
	struct cpufreq_policy *policy = cpufreq_cpu_get(cpu);

	if (!policy)
		return -EINVAL;

	down(&policy->lock);
	ret = __cpufreq_governor(policy, event);
	up(&policy->lock);

	cpufreq_cpu_put(policy);

	return ret;
}
EXPORT_SYMBOL_GPL(cpufreq_governor);


int cpufreq_register_governor(struct cpufreq_governor *governor)
{
	struct cpufreq_governor *t;

	if (!governor)
		return -EINVAL;

	if (!strnicmp(governor->name,"powersave",CPUFREQ_NAME_LEN))
		return -EBUSY;
	if (!strnicmp(governor->name,"performance",CPUFREQ_NAME_LEN))
		return -EBUSY;

	down(&cpufreq_governor_sem);
	
	list_for_each_entry(t, &cpufreq_governor_list, governor_list) {
		if (!strnicmp(governor->name,t->name,CPUFREQ_NAME_LEN)) {
			up(&cpufreq_governor_sem);
			return -EBUSY;
		}
	}
	list_add(&governor->governor_list, &cpufreq_governor_list);

 	up(&cpufreq_governor_sem);

	return 0;
}
EXPORT_SYMBOL_GPL(cpufreq_register_governor);


void cpufreq_unregister_governor(struct cpufreq_governor *governor)
{
	/* backport info: 
	 * As the module usage count isn't assured in 2.4., check for removal
	 * of running cpufreq governor
	 */
	unsigned int i;

	if (!governor)
		return;

	down(&cpufreq_governor_sem);

	for (i=0; i<NR_CPUS; i++) {
		struct cpufreq_policy *policy = cpufreq_cpu_get(i);
		if (!policy)
			goto done;
		down(&policy->lock);

		if (policy->policy != CPUFREQ_POLICY_GOVERNOR)
			goto unlock_done;
		if (policy->governor != governor)
			goto unlock_done;

		/* stop old one, start performance [always present] */
		__cpufreq_governor(policy, CPUFREQ_GOV_STOP);
		policy->policy = CPUFREQ_POLICY_PERFORMANCE;
		__cpufreq_governor(policy, CPUFREQ_GOV_START);

	unlock_done:
		up(&policy->lock);
	done:
		cpufreq_cpu_put(policy);
	}
	list_del(&governor->governor_list);
	up(&cpufreq_governor_sem);
	return;
}
EXPORT_SYMBOL_GPL(cpufreq_unregister_governor);



/*********************************************************************
 *                          POLICY INTERFACE                         *
 *********************************************************************/

/**
 * cpufreq_get_policy - get the current cpufreq_policy
 * @policy: struct cpufreq_policy into which the current cpufreq_policy is written
 *
 * Reads the current cpufreq policy.
 */
int cpufreq_get_policy(struct cpufreq_policy *policy, unsigned int cpu)
{
	struct cpufreq_policy *cpu_policy;
	if (!policy)
		return -EINVAL;

	cpu_policy = cpufreq_cpu_get(cpu);
	if (!cpu_policy)
		return -EINVAL;

	down(&cpu_policy->lock);
	memcpy(policy, cpu_policy, sizeof(struct cpufreq_policy));
	up(&cpu_policy->lock);

	cpufreq_cpu_put(cpu_policy);

	return 0;
}
EXPORT_SYMBOL(cpufreq_get_policy);


/**
 *	cpufreq_set_policy - set a new CPUFreq policy
 *	@policy: policy to be set.
 *
 *	Sets a new CPU frequency and voltage scaling policy.
 */
int cpufreq_set_policy(struct cpufreq_policy *policy)
{
	int ret = 0;
	struct cpufreq_policy *data;

	if (!policy)
		return -EINVAL;

	data = cpufreq_cpu_get(policy->cpu);
	if (!data)
		return -EINVAL;

	/* lock this CPU */
	down(&data->lock);

	memcpy(&policy->cpuinfo, 
	       &data->cpuinfo, 
	       sizeof(struct cpufreq_cpuinfo));

	/* verify the cpu speed can be set within this limit */
	ret = cpufreq_driver->verify(policy);
	if (ret)
		goto error_out;

	down_read(&cpufreq_notifier_rwsem);

	/* adjust if necessary - all reasons */
	notifier_call_chain(&cpufreq_policy_notifier_list, CPUFREQ_ADJUST,
			    policy);

	/* adjust if necessary - hardware incompatibility*/
	notifier_call_chain(&cpufreq_policy_notifier_list, CPUFREQ_INCOMPATIBLE,
			    policy);

	/* verify the cpu speed can be set within this limit,
	   which might be different to the first one */
	ret = cpufreq_driver->verify(policy);
	if (ret) {
		up_read(&cpufreq_notifier_rwsem);
		goto error_out;
	}

	/* notification of the new policy */
	notifier_call_chain(&cpufreq_policy_notifier_list, CPUFREQ_NOTIFY,
			    policy);

	up_read(&cpufreq_notifier_rwsem);

	data->min    = policy->min;
	data->max    = policy->max;

	if (cpufreq_driver->setpolicy) {
		data->policy = policy->policy;
		ret = cpufreq_driver->setpolicy(policy);
	} else {
		if ((policy->policy != data->policy) || 
		    ((policy->policy == CPUFREQ_POLICY_GOVERNOR) && (policy->governor != data->governor))) {
			/* save old, working values */
			unsigned int old_pol = data->policy;
			struct cpufreq_governor *old_gov = data->governor;

			/* end old governor */
			__cpufreq_governor(data, CPUFREQ_GOV_STOP);

			/* start new governor */
			data->policy = policy->policy;
			data->governor = policy->governor;
			if (__cpufreq_governor(data, CPUFREQ_GOV_START)) {
				/* new governor failed, so re-start old one */
				data->policy = old_pol;
				data->governor = old_gov;
				__cpufreq_governor(data, CPUFREQ_GOV_START);
			}
			/* might be a policy change, too, so fall through */
		}
		__cpufreq_governor(data, CPUFREQ_GOV_LIMITS);
	}

 error_out:
	up(&data->lock);
	cpufreq_cpu_put(data);

	return ret;
}
EXPORT_SYMBOL(cpufreq_set_policy);



/*********************************************************************
 *            EXTERNALLY AFFECTING FREQUENCY CHANGES                 *
 *********************************************************************/

/**
 * adjust_jiffies - adjust the system "loops_per_jiffy"
 *
 * This function alters the system "loops_per_jiffy" for the clock
 * speed change. Note that loops_per_jiffy cannot be updated on SMP
 * systems as each CPU might be scaled differently. So, use the arch 
 * per-CPU loops_per_jiffy value wherever possible.
 */
#ifndef CONFIG_SMP
static unsigned long l_p_j_ref = 0;
static unsigned int  l_p_j_ref_freq = 0;

static inline void adjust_jiffies(unsigned long val, struct cpufreq_freqs *ci)
{
	if (!l_p_j_ref_freq) {
		l_p_j_ref = loops_per_jiffy;
		l_p_j_ref_freq = ci->old;
	}
	if ((val == CPUFREQ_PRECHANGE  && ci->old < ci->new) ||
	    (val == CPUFREQ_POSTCHANGE && ci->old > ci->new))
		loops_per_jiffy = cpufreq_scale(l_p_j_ref, l_p_j_ref_freq, ci->new);
}
#else
#define adjust_jiffies(x...) do {} while (0)
#endif


/**
 * cpufreq_notify_transition - call notifier chain and adjust_jiffies on frequency transition
 *
 * This function calls the transition notifiers and the "adjust_jiffies" function. It is called
 * twice on all CPU frequency changes that have external effects. 
 */
void cpufreq_notify_transition(struct cpufreq_freqs *freqs, unsigned int state)
{
	down_read(&cpufreq_notifier_rwsem);
	switch (state) {
	case CPUFREQ_PRECHANGE:
		notifier_call_chain(&cpufreq_transition_notifier_list, CPUFREQ_PRECHANGE, freqs);
		adjust_jiffies(CPUFREQ_PRECHANGE, freqs);
		break;
	case CPUFREQ_POSTCHANGE:
		adjust_jiffies(CPUFREQ_POSTCHANGE, freqs);
		notifier_call_chain(&cpufreq_transition_notifier_list, CPUFREQ_POSTCHANGE, freqs);
		cpufreq_cpu_data[freqs->cpu]->cur = freqs->new;
		break;
	}
	up_read(&cpufreq_notifier_rwsem);
}
EXPORT_SYMBOL_GPL(cpufreq_notify_transition);



/*********************************************************************
 *               REGISTER / UNREGISTER CPUFREQ DRIVER                *
 *********************************************************************/

/**
 * cpufreq_register_driver - register a CPU Frequency driver
 * @driver_data: A struct cpufreq_driver containing the values#
 * submitted by the CPU Frequency driver.
 *
 *   Registers a CPU Frequency driver to this core code. This code 
 * returns zero on success, -EBUSY when another driver got here first
 * (and isn't unregistered in the meantime). 
 *
 */
int cpufreq_register_driver(struct cpufreq_driver *driver_data)
{
	unsigned long flags;
	unsigned int i;

	if (!driver_data || !driver_data->verify || !driver_data->init ||
	    ((!driver_data->setpolicy) && (!driver_data->target)))
		return -EINVAL;

	spin_lock_irqsave(&cpufreq_driver_lock, flags);
	if (cpufreq_driver) {
		spin_unlock_irqrestore(&cpufreq_driver_lock, flags);
		return -EBUSY;
	}
	cpufreq_driver = driver_data;
	spin_unlock_irqrestore(&cpufreq_driver_lock, flags);

	for (i=0; i<NR_CPUS; i++) {
		if (cpu_online(i)) 
			cpufreq_add_dev(i);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(cpufreq_register_driver);


/**
 * cpufreq_unregister_driver - unregister the current CPUFreq driver
 *
 *    Unregister the current CPUFreq driver. Only call this if you have 
 * the right to do so, i.e. if you have succeeded in initialising before!
 * Returns zero if successful, and -EINVAL if the cpufreq_driver is
 * currently not initialised.
 */
int cpufreq_unregister_driver(struct cpufreq_driver *driver)
{
	unsigned long flags;
	unsigned int i;

	if (!cpufreq_driver || (driver != cpufreq_driver))
		return -EINVAL;

	for (i=0; i<NR_CPUS; i++) {
		if (cpu_online(i)) 
			cpufreq_remove_dev(i);
	}

	spin_lock_irqsave(&cpufreq_driver_lock, flags);
	cpufreq_driver = NULL;
	spin_unlock_irqrestore(&cpufreq_driver_lock, flags);

	return 0;
}
EXPORT_SYMBOL_GPL(cpufreq_unregister_driver);
