#include "pin.H"
#include <iostream>

// DBI anti-evasion + hooks
#include "config.h"
#include "memory.h"
#include "hooks.h"
#include "fpu.h"
#include "logging.h"

// libdft
#include "libdft/libdft_config.h"
#include "libdft/bridge.h"
#include "libdft/libdft_api.h"
#include "libdft/tagmap.h"

// ntdll.cpp: map between ordinal and syscall name
extern CHAR* syscallIDs[MAXSYSCALLS];
VOID EnumSyscalls();

// we use TLS for now only to track state across syscall enter and exit
TLS_KEY tls_key = INVALID_TLS_KEY;

#if USE_KNOBS
BOOL _nxKnob;
BOOL _paranoidKnob;
BOOL _rwKnob;
BOOL _leakKnob;
BOOL _libdftKnob;
#endif

KNOB <BOOL> KnobNX(KNOB_MODE_WRITEONCE, "pintool",
	"nx", "false", "enable NX protection");

KNOB <BOOL> KnobParanoid(KNOB_MODE_WRITEONCE, "pintool",
	"paranoid", "false", "enable NX full protection");

KNOB <BOOL> KnobRW(KNOB_MODE_WRITEONCE, "pintool",
	"read_write", "false", "enable read/write protection");

KNOB <BOOL> KnobLeak(KNOB_MODE_WRITEONCE, "pintool",
	"leak", "false", "enable FPU context protection");

KNOB <BOOL> KnobLibdft(KNOB_MODE_WRITEONCE, "pintool",
	"libdft", "false", "enable libdft (for overheads from paper)");

INT32 Usage() {
	cout << "I will protect you from the nasty world out there :-)\n" << endl;

	cout << KNOB_BASE::StringKnobSummary() << endl;

	return -1;
}

VOID antiDBIEvasionConfig() {
	SokLogging::Init();

#if USE_KNOBS
	_leakKnob = KnobLeak.Value();
	_rwKnob = KnobRW.Value();
	_nxKnob = KnobNX.Value();
	_paranoidKnob = KnobParanoid.Value();
	_libdftKnob = KnobLibdft.Value();
#endif

	if (_rwKnob || _nxKnob) {
		// obtain a TLS key
		tls_key = PIN_CreateThreadDataKey(NULL);
		if (tls_key == INVALID_TLS_KEY) {
			LOG_AR("Cannot initialize TLS");
			PIN_ExitProcess(1);
		}
	}
}

VOID FiniCallback(INT32 code, VOID *v) {
	SokLogging::Shutdown();
}

VOID OnThreadStart(THREADID tid, CONTEXT *ctxt, INT32, VOID *) {
	if (_rwKnob || _nxKnob) {
		HOOKS_SetTLSKey(tid);
		MEMORY_OnThreadStart(ctxt);
	}

	if (_libdftKnob) {
		thread_ctx_t *thread_ctx = libdft_thread_start(ctxt);
		#define TTINFO(field) thread_ctx->ttinfo.field
		TTINFO(tid) = tid;
		TTINFO(os_tid) = PIN_GetTid();
		char tmp[32];
		sprintf(tmp, "tainted-%u.log", TTINFO(os_tid));
		TTINFO(logname) = strdup(tmp);
		#undef TTINFO
	}
}

VOID OnThreadFini(THREADID tid, const CONTEXT *ctxt, INT32, VOID *) {
	if (_libdftKnob) {
		libdft_thread_fini(ctxt);
	}
}

VOID Instruction(INS ins, VOID *v) {
	if (_rwKnob || _nxKnob)
		MEMORY_InstrumentINS(ins);
	if (_leakKnob)
		FPU_InstrumentINS(ins);
	// if you want to get an alert when tainted data is met
	//if (_libdftKnob) instrumentForTaintCheck(ins, NULL);
}

VOID Image(IMG img, VOID* v) {
	if (_rwKnob || _nxKnob) {
		MEMORY_LoadImage(img);
	}
}

VOID ImageUnload(IMG img, VOID* v) {
	if (_rwKnob || _nxKnob) {
		MEMORY_UnloadImage(img);
	}
}

VOID SyscallEntry(THREADID thread_id, CONTEXT *ctx, SYSCALL_STANDARD std, void *v) {
	if (_rwKnob || _nxKnob) {
		HOOKS_SyscallEntry(thread_id, ctx, std);
	}
}

VOID SyscallExit(THREADID thread_id, CONTEXT *ctx, SYSCALL_STANDARD std, void *v) {
	if (_rwKnob || _nxKnob) {
		HOOKS_SyscallExit(thread_id, ctx, std);
	}
}


// used for debugging purposes
/*
VOID CONTEXT_ChangeContext(THREADID threadIndex, CONTEXT_CHANGE_REASON reason, const CONTEXT * ctxtFrom, CONTEXT * ctxtTo, INT32 info) {
	if (reason == CONTEXT_CHANGE_REASON_EXCEPTION) {
		ADDRINT _eip;
		PIN_GetContextRegval(ctxtFrom, REG_INST_PTR, reinterpret_cast<UINT8*>(&_eip));
		// [...] do what you need to do when debugging :-)
	}
}

VOID OnContextChange(THREADID threadIndex, CONTEXT_CHANGE_REASON reason, const CONTEXT *ctxtFrom, CONTEXT *ctxtTo, INT32 info, VOID *v) {
	CONTEXT_ChangeContext(threadIndex, reason, ctxtFrom, ctxtTo, info);
}

EXCEPT_HANDLING_RESULT InternalExceptionHandler(THREADID tid, EXCEPTION_INFO *pExceptInfo, PHYSICAL_CONTEXT *pPhysCtxt, VOID *v) {

	cout << PIN_ExceptionToString(pExceptInfo).c_str() <<
		" Code: " << pExceptInfo->GetExceptCode() << endl; // TODO use macro to print

	return EHR_CONTINUE_SEARCH;
}
*/


int main(int argc, char *argv[]) {

	PIN_InitSymbols();
	if (PIN_Init(argc, argv)) {
		return Usage();
	}

	// initialize some stuff
	antiDBIEvasionConfig();

	if (_rwKnob || _nxKnob) {
		MEMORY_Init();
	}
	if (_leakKnob) {
		FPU_Init();
	}

	// syscall instrumentation
	EnumSyscalls(); // parse ntdll for ordinals
	PIN_AddSyscallEntryFunction(SyscallEntry, NULL);
	PIN_AddSyscallExitFunction(SyscallExit, NULL);

	// anti-evasion syscall hooks
	HOOKS_Init();

	// INS instrumentation
	INS_AddInstrumentFunction(Instruction, NULL);

	// IMG instrumentation
	IMG_AddInstrumentFunction(Image, NULL);
	IMG_AddUnloadFunction(ImageUnload, NULL);

	// use for debugging purposes
	//PIN_AddContextChangeFunction(OnContextChange, NULL);
	//PIN_AddInternalExceptionHandler(InternalExceptionHandler, NULL);

	// events
	PIN_AddThreadStartFunction(OnThreadStart, NULL);
	PIN_AddThreadFiniFunction(OnThreadFini, NULL);

	PIN_AddFiniFunction(FiniCallback, NULL);

	// libdft initialization - we compare overheads in the paper
	if (_libdftKnob) {
		if (libdft_init_data_only()) {
			LOG_AR("Error initializing libdft");
			exit(1);
		}
		TRACE_AddInstrumentFunction(libdft_trace_inspect, NULL);
		// verbose mode for following taint propagation
		//INS_AddInstrumentFunction(instrumentForTaintCheck, NULL);
	}

	PIN_StartProgram();

	return 0;
}
