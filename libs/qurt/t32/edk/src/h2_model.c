/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

void main (void)
{
	static info_command_t tasklist  =
		{   "TASKLIST", "TL",   "TaskList",
		"__EM_TL: List of all TCBs",
		TlParser };

	// Definition for new command "TASK.TraceList"
	static info_command_t tracelist =
		{   "TRACELIST",  "TRACE",   "TraceList",
		"__EM_TRACE: Kernel Traces",
		TraceParser
		};

	T32DefineGetMagic(GetTaskHandle);
	T32DefineCurrentTask(GetCurrTask);
	T32DefineTaskId(MagicToSwtid);
	T32DefineTaskName(MagicToName);
	T32DefineTaskList(TaskList);
	T32DefineGetContext(GetContext);
	T32DefineCommand(&tasklist);
	T32DefineCommand(&tracelist);
}

char *get_qurt_name(unsigned int id)
{
	static char name[16];
	for (ptr = qurt_root; ptr != 0; ptr = ptr->next) {
		if (read_word(ptr+0) == id) {
			copy out name
			return name;
		}
	}
	strcpy(name,"?");
	return name;
}

char *MagicToName(ulong context_ptr)
{
	static char name[64];
	unsigned int id = context_to_id(context_ptr);
	snprintf(name,64,"H2:%08x qurt: <%s>",get_qurt_name(id));
	name[63] = '\0';
	return name;
}

ulong *TaskList()
{
	static ulong contexts[MAX_THREADS];
	int i,j;
	int idx = 0;
	for (i = 0; i < MAX_VMS; i++) {
		for (j = 0; j < vm->ncpus; j++) {
			/* context_ptr = contexts + j*context_size */
			/* check status.  */
			/* If status == DEAD, continue */
			contexts[idx++] = context_ptr;
		}
	}
	return contexts;
}

ulong argvalue[2];

static void state_name(unsigned long me)
{
	static char statetxt[16];
	int state = get_byte(me+offset_state)
	if (state != RUNNING) {
		sprintf(statetxt,"%s"state_name[state]);
	} else {
		sprintf(statetxt,"%s(%d)"state_name[state],get_byte(me+offset_cpuidx));
	}
}

void display_line(unsigned long me)
{
	unsigned int id;
	unsigned int tmp;

	id = context_to_id(me);
	T32Printf("%08x %d %d  %08x  ",
		id,
		(id >> (IGNBITS+CPUIDXBITS)),
		(id >> (IGNBITS)) & ((1<<CPUIDXBITS)-1),
		me);

	T32Printf("%s ",state_name(me));
	T32Printf("<%s>",qurt_name(id));
	T32Printf("\n");
}

void TlDisplayOne(void)
{
	unsigned long me = argvalue[0];
	display_line(me);
}

cmd_display_t *TlInit(void)
{
	static const char header[] = "H2 ID    VM VCPU  ContextAddress State Name";
	static cmd_display_t displayone = { WINDOW, 0, 240, header, TlDisplayOne};
	static cmd_display_t displayall = { WINDOW, 0, 240, header, TlDisplayAll};
	static cmd_display_t errdisp = { MESSAGE, 0, 0, "Sorry, can't get task list", 0 };
}

info_cmd_init_t TlParser(void)
{
	argvalue[0] = T32ParseValue("<task>");
	return TlInit;
}

