/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "Lua.h"
#include <sstream>
#include <map>

extern "C" {
#include "../lua/src/lua.h"
#include "../lua/src/lauxlib.h"
#include "../lua/src/lstate.h"
#include "../lua/src/lobject.h"
#include "../lua/src/lundump.h"
#include "../lua/src/lualib.h"
// undef macros
//#undef val_
//#undef num_
}

namespace Defs
{
	enum Breakpoint
	{
		BPT_NONE	= 0x00,		// no breakpoint
		BPT_EXECUTE	= 0x01,		// breakpoint
		BPT_READ	= 0x02,		// read breakpoint
		BPT_WRITE	= 0x04,		// write breakpoint
		BPT_MASK	= 0x07,
		BPT_NO_CODE	= 0x08,		// no source code
		BPT_TEMP_EXEC=0x10,		// temporary breakpoint
		BPT_DISABLED= 0x80		// breakpoint disabled
	};
}

using namespace lua_details;


struct lua_details::State
{
	State() : step_event_(false, true), start_event_(false, true)
	{
		L = 0;
		lua_ = 0;
		thread_ = 0;
		break_flag_ = abort_flag_ = false;
		func_call_level_ = 0;
		stop_at_level_ = 0;
		run_mode_ = Run;
		is_running_ = false;
		status_ready_ = false;
		init_libs_done_ = false;
		steps_to_execute_ = 0;
		free_cfunc_slot_ = 0;

		L = luaL_newstate();
		if (L == 0)
			throw lua_exception("not enough memory for Lua state");
	}

	~State()
	{
		if (thread_)
		{
			abort_flag_ = true;
			step_event_.SetEvent();
			start_event_.SetEvent();
			::WaitForSingleObject(*thread_, -1);
			delete thread_;
		}

		lua_close(L);
	}

	enum RunMode { StepInto, StepOver, StepOut, Run, ExecuteNSteps };
	void go(RunMode mode);

	static UINT AFX_CDECL exec_thread(LPVOID param);
	static void exec_hook_function(lua_State* L, lua_Debug* ar);
	void exec_hook(lua_State* L, lua_Debug* dbg);
	void line_hook(lua_State* L, lua_Debug* dbg);
	void count_hook(lua_State* L, lua_Debug* dbg);
	void call_hook(lua_State* L, lua_Debug* dbg);
	void ret_hook(lua_State* L, lua_Debug* dbg);
	void suspend_exec(lua_Debug* dbg, bool forced);

	void notify(Lua::Event ev, lua_Debug* dbg);

	bool is_execution_finished() const		{ return ::WaitForSingleObject(*thread_, 0) != WAIT_TIMEOUT; }
	bool is_data_available() const;

	bool breakpoint_at_line(int line) const;
	bool toggle_breakpoint(int line);

	void init_libs();	// initialize Lua built-in libraries

	lua_State* L;
	Lua* lua_;
	CWinThread* thread_;
	CEvent step_event_;
	CEvent start_event_;
	bool abort_flag_;
	bool break_flag_;
	boost::function<void (Lua::Event, int)> callback_;
	std::string status_msg_;
	bool status_ready_;
	typedef std::map<int, Defs::Breakpoint> BreakpointMap;
	BreakpointMap breakpoints_;
	mutable CCriticalSection breakpoints_lock_;
	int func_call_level_;
	int stop_at_level_;
	bool is_running_;
	bool init_libs_done_;
	int steps_to_execute_;
	RunMode run_mode_;
	static const size_t cfunc_slots= 10;
	size_t free_cfunc_slot_;
	boost::function<int (Lua* self)> c_fnunc[cfunc_slots];

private:
	void run();
};


#define GET_STATE_FROM_PARAM(state)		static_cast<State*>((state)->user_param)

#define DEF_CALL_C_FUNC(N)	static int CallCFunc_##N(lua_State* L)	\
{	\
	State* s= GET_STATE_FROM_PARAM(L);	\
	return s->c_fnunc[N](s->lua_);	\
}

DEF_CALL_C_FUNC(0)
DEF_CALL_C_FUNC(1)
DEF_CALL_C_FUNC(2)
DEF_CALL_C_FUNC(3)
DEF_CALL_C_FUNC(4)
DEF_CALL_C_FUNC(5)
DEF_CALL_C_FUNC(6)
DEF_CALL_C_FUNC(7)
DEF_CALL_C_FUNC(8)
DEF_CALL_C_FUNC(9)

static lua_CFunction CFuncs[State::cfunc_slots]=
{
	CallCFunc_0, CallCFunc_1, CallCFunc_2, CallCFunc_3, CallCFunc_4, CallCFunc_5, CallCFunc_6, CallCFunc_7, CallCFunc_8, CallCFunc_9
};


#define PRINT_STACK_SIZE(L)		// TRACE(_T("lua stack: %d elem\n"), (L)->top - (L)->base)


void State::init_libs()	// initialize Lua built-in libraries
{
	lua_gc(L, LUA_GCSTOP, 0);  /* stop collector during initialization */
	luaL_openlibs(L);  /* open libraries */
	lua_gc(L, LUA_GCRESTART, 0);

	lua_sethook(L, &State::exec_hook_function, LUA_MASKLINE | LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);

	init_libs_done_ = true;
}


void State::run()
{
	is_running_ = true;

	if (!init_libs_done_)
		init_libs();

	notify(Lua::Start, 0);

	// correct the level: we are entering function (main chunk) now
	stop_at_level_ = func_call_level_ + 1;

//	lua_sethook(L, &State::exec_hook_function, LUA_MASKLINE | LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);

	int narg= 0;
	//int base= lua_gettop(L) - narg;  // function index
	int err_fn= 0;
	int status= lua_pcall(L, narg, LUA_MULTRET, err_fn);

	if (abort_flag_)
		status_msg_ = "Aborted";
	else if (status)
		status_msg_ = lua_tostring(L, -1);
	else
		status_msg_ = "Finished";

	status_ready_ = true;	// it's ok to read 'status_msg_' from different thread now
	if (!abort_flag_)
		notify(Lua::Finished, 0);
}


std::string Lua::Status() const
{
	if (state_->is_execution_finished() || state_->status_ready_)
		return state_->status_msg_;

	return state_->is_running_ ? "Running" : "Stopped";
}


void Lua::SetCallback(const boost::function<void (Event, int)>& fn)
{
	state_->callback_ = fn;
}

// internal Lua data is only available when VM stopped at a breakpoint
bool State::is_data_available() const
{
	if (is_execution_finished())
		return false;

	return !is_running_;
}


void State::notify(Lua::Event ev, lua_Debug* dbg)
{
	if (callback_)
	{
		try
		{
			callback_(ev, dbg != 0 ? dbg->currentline : -1);
		}
		catch (...)
		{
		}
	}
}


void State::exec_hook_function(lua_State* L, lua_Debug* dbg)
{
	State* state= GET_STATE_FROM_PARAM(L);

	if (state->abort_flag_)
	{
		lua_error(state->L);		// abort now
		return;
	}

	switch (dbg->event)
	{
	case LUA_HOOKCOUNT:
		state->count_hook(L, dbg);
		break;

	case LUA_HOOKCALL:
		state->call_hook(L, dbg);
		break;

	case LUA_HOOKRET:
	case LUA_HOOKTAILCALL:	//verify
		state->ret_hook(L, dbg);
		break;

	case LUA_HOOKLINE:
		state->line_hook(L, dbg);
		break;
	}
}


void State::count_hook(lua_State* L, lua_Debug*)
{
	if (break_flag_)
	{
		lua_Debug dbg;
		memset(&dbg, 0, sizeof(dbg));
		dbg.currentline = -1;

		if (lua_getstack(L, 0, &dbg))
		{
			// retrieve current line number, count hook doesn't provide it
			int stat= lua_getinfo(L, "l", &dbg);
			if (stat == 0)
				dbg.currentline = -1;
		}

		// break signaled; stop
		suspend_exec(&dbg, true);
	}
}


void State::call_hook(lua_State* L, lua_Debug* dbg)
{
	func_call_level_++;
}


void State::ret_hook(lua_State* L, lua_Debug* dbg)
{
	func_call_level_--;
}


void State::line_hook(lua_State* L, lua_Debug* dbg)
{
	if (break_flag_)
	{
		// break signaled; stop
		suspend_exec(dbg, true);
	}
	else if (run_mode_ == ExecuteNSteps)
	{
		if (--steps_to_execute_ < 0)
			suspend_exec(dbg, true);
	}
	else if (run_mode_ == StepOver)
	{
		if (stop_at_level_ >= func_call_level_)	// 'step over' done?
			suspend_exec(dbg, true);	// stop now
	}
	else if (run_mode_ == StepOut)
	{
		if (stop_at_level_ > func_call_level_)	// 'step out' done?
			suspend_exec(dbg, true);	// stop now
	}
	else if (run_mode_ == Run)	// run without delay?
	{
		// check breakpoints
		if (breakpoint_at_line(dbg->currentline))
			suspend_exec(dbg, true);	// stop now; there's a breakpoint at the current line
	}
	else
		suspend_exec(dbg, false);	// line-by-line execution, so stop
}


void State::suspend_exec(lua_Debug* dbg, bool forced)
{
	if (forced)
		step_event_.ResetEvent();

	// step by step execution

	notify(Lua::NewLine, dbg);

	if (::WaitForSingleObject(step_event_, 0) != WAIT_OBJECT_0)		// blocked?
	{
		is_running_ = false;
		CSingleLock wait(&step_event_, true);
	}

	if (abort_flag_)
	{
		lua_error(L);		// abort now
		return;
	}

	is_running_ = true;

	step_event_.ResetEvent();
}


UINT AFX_CDECL State::exec_thread(LPVOID param)
{
	State* state= static_cast<State*>(param);
	try
	{
		CSingleLock wait(&state->start_event_, true);

		if (!state->abort_flag_)
		{
			state->run_mode_ = StepInto;
			state->run();
		}
	}
	catch (...)
	{
		ASSERT(false);
		return 1;
	}
	return 0;
}



Lua::Lua() : state_(new State())
{
	state_->L->user_param = state_.get();
	state_->lua_ = this;

//	lua_sethook(state_->L, &State::exec_hook_function, LUA_MASKLINE | LUA_MASKCALL | LUA_MASKRET, 0);

	state_->thread_ = ::AfxBeginThread(&State::exec_thread, state_.get(), THREAD_PRIORITY_LOWEST, 0, CREATE_SUSPENDED);
	if (state_->thread_ == 0)
		throw lua_exception("cannot create Lua execution thread");
	state_->thread_->m_bAutoDelete = false;
	state_->thread_->ResumeThread();
	PRINT_STACK_SIZE(state_->L);
}


Lua::~Lua()
{}


void Lua::LoadFile(const char* file_path)
{
	if (luaL_loadfile(state_->L, file_path) != 0)
		throw lua_exception(lua_tostring(state_->L, -1));
	PRINT_STACK_SIZE(state_->L);
}


bool Lua::LoadFile(const char* file_path, const char*& msg)
{
	if (luaL_loadfile(state_->L, file_path) != 0)
	{
		msg = lua_tostring(state_->L, -1);
		return false;
	}

	PRINT_STACK_SIZE(state_->L);
	msg = "ok";
	return true;
}


bool Lua::LoadBuffer(const char* source, const char*& msg, const char* name/*= 0*/)
{
	if (luaL_loadbuffer(state_->L, source, strlen(source), name) != 0)
	{
		msg = lua_tostring(state_->L, -1);
		return false;
	}

	PRINT_STACK_SIZE(state_->L);
	msg = "ok";
	return true;
}


static int writer(lua_State* L, const void* p, size_t size, void* u)
{
	UNUSED(L);
	ProgBuf& v= *static_cast<ProgBuf*>(u);
	v.resize(size);
	if (size > 0)
		memcpy(&v.front(), p, size);
	return size != 0;
}


class LuaLocker
{
public:
	LuaLocker(lua_State* L) : L_(L)
	{
		lua_lock(L_);
	}

	~LuaLocker()
	{
		lua_unlock(L_);
	}

private:
	lua_State* L_;
};


class pop_stack_elements
{
public:
	pop_stack_elements(lua_State* L, int num) : L_(L), number_(num)
	{}

	void dec()
	{
		--number_;
	}

	~pop_stack_elements()
	{
		try
		{
			if (number_ > 0)
				lua_pop(L_, number_);
		}
		catch (...)
		{}
	}

private:
	lua_State* L_;
	int number_;
};


void Lua::Dump(ProgBuf& program, bool debug)
{
	program.clear();

	lua_TValue* t= state_->L->top - 1;

	if (!ttisfunction(t))
		return;

	const Proto* f= clvalue(t)->l.p; //toproto(state_->L, -1);

	{
		LuaLocker lock(state_->L);
//export this fn:
//		luaU_dump(state_->L, f, writer, &program, !debug);
	}
}


int Lua::Call(int num_arg, const char** msg)
{
	int err_fn= 0;
	int status= lua_pcall(state_->L, num_arg, 1/*LUA_MULTRET*/, err_fn);

	if (status != 0 && msg != 0)
		*msg = lua_tostring(state_->L, -1);

	PRINT_STACK_SIZE(state_->L);
	return status;
}


int Lua::Call(int num_arg, int max_steps_to_execute, const char** msg)
{
	state_->steps_to_execute_ = max_steps_to_execute;
	state_->run_mode_ = State::ExecuteNSteps;

	int err_fn= 0;
	int status= lua_pcall(state_->L, num_arg, 1/*LUA_MULTRET*/, err_fn);

	if (status != 0 && msg != 0)
		*msg = lua_tostring(state_->L, -1);

	PRINT_STACK_SIZE(state_->L);
	return status;
}


void State::go(RunMode mode)
{
	if (is_execution_finished())
		return;

	run_mode_ = mode;
	break_flag_ = false;
	stop_at_level_ = func_call_level_;

	if (::WaitForSingleObject(start_event_, 0) == WAIT_OBJECT_0)
		step_event_.SetEvent();
	else
	{
		start_event_.SetEvent();
		if (mode == Run)
			step_event_.SetEvent();
	}

	if (mode == Run)	//TODO: improve
		notify(Lua::Running, 0);
}


void Lua::Run()
{
	state_->go(State::Run);
}


void Lua::StepInto()
{
	state_->go(State::StepInto);
}


void Lua::StepOver()
{
	state_->go(State::StepOver);
}


void Lua::StepOut()
{
	state_->go(State::StepOut);
}

// execute up to given amount of steps (lines) and stop
void Lua::ExecuteNSteps(int steps)
{
	state_->steps_to_execute_ = steps;
	state_->go(State::ExecuteNSteps);
}


bool State::breakpoint_at_line(int line) const
{
	CSingleLock lock(&breakpoints_lock_, true);

	BreakpointMap::const_iterator it= breakpoints_.find(line);

	if (it == breakpoints_.end())
		return false;

	return (it->second & Defs::BPT_MASK) == Defs::BPT_EXECUTE;
}


bool State::toggle_breakpoint(int line)
{
	CSingleLock lock(&breakpoints_lock_, true);

	if (breakpoint_at_line(line))
	{
		breakpoints_.erase(line);
		return false;
	}
	else
	{
		breakpoints_[line] = Defs::BPT_EXECUTE;
		return true;
	}
}


bool Lua::ToggleBreakpoint(int line)
{
	return state_->toggle_breakpoint(line);
}


void Lua::Break()
{
	state_->break_flag_ = true;
}


bool Lua::IsRunning() const
{
	if (state_->is_execution_finished())
		return false;

	return state_->is_running_;
}

bool Lua::IsFinished() const
{
	return state_->is_execution_finished();
}

bool Lua::IsStopped() const
{
	return state_->is_data_available();
}


std::string Lua::GetCallStack() const
{
	if (!state_->is_data_available())
		return std::string();

	std::ostringstream callstack;

	// local info= debug.getinfo(1)
//LUA_API int lua_getstack (lua_State *L, int level, lua_Debug *ar);

	int level= 0;
	lua_Debug dbg;
	memset(&dbg, 0, sizeof dbg);

	while (lua_getstack(state_->L, level++, &dbg))
	{
		if (lua_getinfo(state_->L, "Snl", &dbg) == 0)
		{
			callstack << "-- error at level " << level;
			break;
		}

		callstack << dbg.short_src;
		if (dbg.currentline > 0)
			callstack << ':' << dbg.currentline;

		if (*dbg.namewhat != '\0')  /* is there a name? */
			callstack << " in function " << dbg.name;
		else
		{
			if (*dbg.what == 'm')  /* main? */
				callstack << " in main chunk";
			else if (*dbg.what == 'C' || *dbg.what == 't')
				callstack << " ?";  /* C function or tail call */
			else
				callstack << " in file <" << dbg.short_src << ':' << dbg.linedefined << '>';
		}

		callstack << std::endl;
	}


	return callstack.str();
}


static char* to_pointer(char* buffer, const void* ptr)
{
	sprintf(buffer, "0x%p", ptr);
	return buffer;
}

static std::string to_table(lua_State* L, int index)
{
//	lua_gettable
}

void capture_value(lua_State* L, Lua::Value& v, int index, int recursive= 0, size_t table_size_limit= 10);

static bool list_table(lua_State* L, int idx, Lua::TableInfo& out, int recursive= 0)
{
	out.clear();

	if (lua_type(L, idx) != LUA_TTABLE)
		return false;

	size_t size= lua_rawlen(L, idx);

	out.reserve(size);

	// table to traverse
	lua_pushvalue(L, idx);

	// push a key
	lua_pushnil(L);

	pop_stack_elements pop(L, 2);	// remove key & table off the stack at the end of this fn
//	pop_stack_elements pop(L, 1);	// remove table off the stack at the end of this fn

	int table= lua_gettop(L) - 1;

	// traverse a table
	while (lua_next(L, table))
	{
		pop_stack_elements pop(L, 1);

		Lua::Field field;
		capture_value(L, field.key, -2);
		capture_value(L, field.val, -1, recursive);

		out.push_back(field);
	}

	pop.dec();	// final lua_next call removed key

	return true;
}


static std::string table_as_string(const Lua::TableInfo& table, size_t limit)
{
	std::ostringstream ost;

	ost << "{ ";

	const size_t count= table.size();

	for (size_t i= 0; i < count; ++i)
	{
		const Lua::Field& f= table[i];

		if (i > 0)
			ost << ", ";

		ost << f.key.value << " = " << f.val.value;

		if (i + 1 == limit)
		{
			ost << ", ... ";
			break;
		}
	}

	if (count > 0)
		ost << ' ';

	ost << "}";

	return ost.str();
}


// store information about Lua value present at the 'index' inside 'v' struct
void capture_value(lua_State* L, Lua::Value& v, int index, int recursive, size_t table_size_limit)
{
	int i= index;
	char buf[100];	// temp output for fast number/pointer formatting

	int t= lua_type(L, i);

	switch (t)
	{
	case LUA_TSTRING:
		v.type = Lua::String;
		v.value = lua_tostring(L, i);
		break;

	case LUA_TBOOLEAN:
		v.type = Lua::Bool;
		v.value = lua_toboolean(L, i) ? "true" : "false";
		break;

	case LUA_TNUMBER:
		v.type = Lua::Number;
		sprintf(buf, "%g", static_cast<double>(lua_tonumber(L, i)));
		v.value = buf;
		break;

	case LUA_TLIGHTUSERDATA:
		v.type = Lua::LightUserData;
		v.value = to_pointer(buf, lua_topointer(L, i));
		break;

	case LUA_TUSERDATA:
		v.type = Lua::UserData;
		v.value = to_pointer(buf, lua_topointer(L, i));
		break;

	case LUA_TTABLE:
		v.type = Lua::Table;
		if (recursive > 0)
		{
			Lua::TableInfo t;
			list_table(L, i, t, recursive - 1);
			v.value = table_as_string(t, table_size_limit);
		}
		else
			v.value = to_pointer(buf, lua_topointer(L, i));
		break;

	case LUA_TFUNCTION:
		v.type = Lua::Function;
		v.value = to_pointer(buf, lua_topointer(L, i));
		break;

	case LUA_TTHREAD:
		v.type = Lua::Thread;
		v.value = to_pointer(buf, lua_topointer(L, i));
		break;

	case LUA_TNIL:
		v.type = Lua::Nil;
		v.value.clear();
		break;

	default:
		v.type = Lua::None;
		v.value.clear();
		break;
	}

	v.type_name = lua_typename(L, t);
}


static bool list_virtual_stack(lua_State* L, Lua::ValueStack& stack, size_t table_size_limit)
{
	int size= lua_gettop(L);

	stack.clear();
	stack.reserve(size);

	for (int idx= size - 1; idx > 0; --idx)
	{
		Lua::Value v;
		capture_value(L, v, idx, 1, table_size_limit);

		stack.push_back(v);
	}

	return true;
}


bool Lua::GetLocalVars(std::vector<Var>& out, int level) const
{
	out.clear();

	if (!state_->is_data_available())
		return false;

	lua_Debug dbg;
	memset(&dbg, 0, sizeof(dbg));

	if (!lua_getstack(state_->L, level, &dbg))
		return false;

	if (lua_getinfo(state_->L, "Snl", &dbg))
	{
		const int SAFETY_COUNTER= 10000;

		for (int i= 1; i < SAFETY_COUNTER; ++i)
		{
			const char* name= lua_getlocal(state_->L, &dbg, i);

			if (name == 0)
				break;

			pop_stack_elements pop(state_->L, 1);	// pop variable value eventually

			Var var;
			var.name = name;
			capture_value(state_->L, var.v, lua_gettop(state_->L));

			out.push_back(var);
		}
	}

	return true;
}


//bool Lua::GetGlobalVars(TableInfo& out, bool deep) const
//{
//	if (!state_->is_data_available())
//		return false;
//
//	return list_table(state_->L, LUA_GLOBALSINDEX, out, deep ? 1 : 0);
//}


bool Lua::GetValueStack(ValueStack& stack) const
{
	if (!state_->is_data_available())
		return false;

	const size_t limit_table_elements_to= 10;

	return list_virtual_stack(state_->L, stack, limit_table_elements_to);
}


// fill stack frame variable with info from Lua debug struct
static void fill_frame(const lua_Debug& dbg, Lua::StackFrame& frame)
{
	frame.Clear();

	frame.source = dbg.source ? dbg.source : dbg.short_src;

	if (dbg.currentline > 0)
		frame.current_line = dbg.currentline;

	if (dbg.what)
	{
		if (strcmp(dbg.what, "C") == 0)
			frame.type = Lua::StackFrame::CFun;
		else if (strcmp(dbg.what, "Lua") == 0)
			frame.type = Lua::StackFrame::LuaFun;
		else if (strcmp(dbg.what, "main") == 0)
			frame.type = Lua::StackFrame::MainChunk;
		else if (strcmp(dbg.what, "tail") == 0)
			frame.type = Lua::StackFrame::TailCall;
		else
			frame.type = Lua::StackFrame::Err;
	}

	if (dbg.namewhat != 0 && *dbg.namewhat != '\0')	// is there a name?
		frame.name_what = dbg.name;

	frame.last_line_defined = dbg.lastlinedefined;
	frame.line_defined = dbg.linedefined;
}


Lua::StackFrame::StackFrame()
{
	Clear();
}


void Lua::StackFrame::Clear()
{
	 current_line = 0;
	 type = Err;
	 line_defined = last_line_defined = 0;
}


const char* Lua::StackFrame::SourcePath() const
{
	if (source.size() > 1 && source[0] == '@')
		return source.c_str() + 1;

	return 0;
}



bool Lua::GetCallStack(CallStack& stack) const
{
	if (!state_->is_data_available())
		return false;

	stack.clear();
	stack.reserve(8);

	int level= 0;
	lua_Debug dbg;
	memset(&dbg, 0, sizeof dbg);

	while (lua_getstack(state_->L, level++, &dbg))
	{
		StackFrame frame;

		if (lua_getinfo(state_->L, "Snl", &dbg) == 0)
		{
			stack.push_back(frame);	// error encountered
			break;
		}

		fill_frame(dbg, frame);

		stack.push_back(frame);
	}

	return true;
}


bool Lua::GetCurrentSource(StackFrame& top) const
{
	if (!state_->is_data_available())
		return false;

	int level= 0;
	lua_Debug dbg;
	memset(&dbg, 0, sizeof dbg);

	if (!lua_getstack(state_->L, level, &dbg) || !lua_getinfo(state_->L, "Snl", &dbg))
		return false;

	fill_frame(dbg, top);

	return true;
}



void Lua::SetStrict()
{
	ASSERT(!IsRunning());

	PRINT_STACK_SIZE(state_->L);

	if (!state_->init_libs_done_)
		state_->init_libs();

/*
-- strict.lua
-- checks uses of undeclared global variables
-- All global variables must be 'declared' through a regular assignment
-- (even assigning nil will do) in a main chunk before being used
-- anywhere or assigned to inside a function.
*/
#define eol "\r\n";

	const char* strict=
"local getinfo, error, rawset, rawget = debug.getinfo, error, rawset, rawget" eol

"local mt = getmetatable(_G)" eol
"if mt == nil then" eol
"  mt = {}" eol
"  setmetatable(_G, mt)" eol
"end" eol

"mt.__declared = {}" eol

"local function what ()" eol
"  local d = getinfo(3, \"S\")" eol
"  return d and d.what or \"C\"" eol
"end" eol

"mt.__newindex = function (t, n, v)" eol
"  if not mt.__declared[n] then" eol
"    local w = what()" eol
"    if w ~= \"main\" and w ~= \"C\" then" eol
"      error(\"assign to undeclared variable '\"..n..\"'\", 2)" eol
"    end" eol
"    mt.__declared[n] = true" eol
"  end" eol
"  rawset(t, n, v)" eol
"end" eol
  
"mt.__index = function (t, n)" eol
"  if not mt.__declared[n] and what() ~= \"C\" then" eol
"    error(\"variable '\"..n..\"' is not declared\", 2)" eol
"  end" eol
"  return rawget(t, n)" eol
"end" eol;

#undef eol

	const char* msg= 0;
	if (!LoadBuffer(strict, msg))
	{
		ASSERT(false);
		return;
	}

	VERIFY(Call(0) == 0);

	PopTop();

	PRINT_STACK_SIZE(state_->L);
}


void Lua::NewTable()
{
	ASSERT(!IsRunning());
	lua_newtable(state_->L);
	PRINT_STACK_SIZE(state_->L);
}


void Lua::SetMetaTable()
{
	ASSERT(!IsRunning());
	lua_setmetatable(state_->L, -2);
	PRINT_STACK_SIZE(state_->L);
}


void Lua::GetMetaTable()
{
	ASSERT(!IsRunning());
	lua_getmetatable(state_->L, -1);
	PRINT_STACK_SIZE(state_->L);
}


void Lua::PushValue(const char* str)
{
	ASSERT(!IsRunning());
	lua_pushstring(state_->L, str);
	PRINT_STACK_SIZE(state_->L);
}

void Lua::PushValue(const std::string& str)
{
	PushValue(str.c_str());
}

void Lua::PushValue(double d)
{
	ASSERT(!IsRunning());
	lua_pushnumber(state_->L, d);
	PRINT_STACK_SIZE(state_->L);
}

void Lua::PushValue(size_t n)
{
	PushValue(static_cast<double>(n));
}

void Lua::PushValue(int n)
{
	PushValue(static_cast<double>(n));
}

#ifdef _WIN64
void Lua::PushValue(unsigned int n)
{
	PushValue(static_cast<double>(n));
}
#endif

void Lua::PushValue(bool b)
{
	ASSERT(!IsRunning());
	lua_pushboolean(state_->L, b);
	PRINT_STACK_SIZE(state_->L);
}


void Lua::PushNil()
{
	ASSERT(!IsRunning());
	lua_pushnil(state_->L);
	PRINT_STACK_SIZE(state_->L);
}


void Lua::PushUserValue(void* user_data)
{
	ASSERT(!IsRunning());
	lua_pushlightuserdata(state_->L, user_data);
	PRINT_STACK_SIZE(state_->L);
}


void Lua::GetField(const char* key, int tbl_idx)
{
	ASSERT(!IsRunning());
	lua_getfield(state_->L, tbl_idx, key);
	PRINT_STACK_SIZE(state_->L);
}


void Lua::SetField(const char* key, double value)
{
	ASSERT(!IsRunning());
	lua_pushnumber(state_->L, value);
	lua_setfield(state_->L, -2, key);
	PRINT_STACK_SIZE(state_->L);
}


void Lua::SetField(const char* key, const char* value)
{
	ASSERT(!IsRunning());
	lua_pushstring(state_->L, value);
	lua_setfield(state_->L, -2, key);
	PRINT_STACK_SIZE(state_->L);
}


void Lua::SetFieldUser(const char* key, void* user_data)
{
	ASSERT(!IsRunning());
	lua_pushlightuserdata(state_->L, user_data);
	lua_setfield(state_->L, -2, key);
	PRINT_STACK_SIZE(state_->L);
}


void Lua::SetField(const char* key, const boost::function<int (Lua* self)>& fn)
{
	ASSERT(!IsRunning());
	if (state_->free_cfunc_slot_ >= State::cfunc_slots)
		throw lua_exception("no more function slots available");

	state_->c_fnunc[state_->free_cfunc_slot_] = fn;

	lua_pushcfunction(state_->L, CFuncs[state_->free_cfunc_slot_++]);
	lua_setfield(state_->L, -2, key);
	PRINT_STACK_SIZE(state_->L);
}


void Lua::SetField(const char* key)	// use value at the top of the stack
{
	ASSERT(!IsRunning());
	lua_setfield(state_->L, -2, key);
	PRINT_STACK_SIZE(state_->L);
}


void Lua::SetGlobal(const char* name)
{
	ASSERT(!IsRunning());
	lua_setglobal(state_->L, name);
	PRINT_STACK_SIZE(state_->L);
}


void Lua::GetGlobal(const char* name)
{
	ASSERT(!IsRunning());
	lua_getglobal(state_->L, name);
	PRINT_STACK_SIZE(state_->L);
}


std::string Lua::GetTop() const
{
	const char* text= StackPeek(-1, true);
	return text ? std::string(text) : std::string();
}


void* Lua::GetUserData(int idx)
{
	ASSERT(!IsRunning());
	void* p= lua_touserdata(state_->L, idx);
	PRINT_STACK_SIZE(state_->L);
	return p;
}


const char* Lua::StackPeek(int index, bool translate) const
{
	ASSERT(!IsRunning());

	if (!translate)
		return lua_tostring(state_->L, index);

	int t= lua_type(state_->L, index);
	const char* text= 0;

	switch (t)
	{
	case LUA_TBOOLEAN:
		text = lua_toboolean(state_->L, index) ? "true" : "false";
		break;

	case LUA_TNUMBER:
	case LUA_TSTRING:
		text = lua_tostring(state_->L, index);
		break;

	case LUA_TNIL:
		text = "nil";
		break;

	default:
		text = lua_typename(state_->L, t);
		break;
	}

	return text;
}


bool Lua::GetTop(double& val) const
{
	ASSERT(!IsRunning());
	int top= -1;
	if (lua_isnumber(state_->L, top))
	{
		val = lua_tonumber(state_->L, top);
		return true;
	}
	else
		return false;
}


bool Lua::GetTop(bool& val) const
{
	ASSERT(!IsRunning());
	int top= -1;
	if (lua_isboolean(state_->L, top))
	{
		val = !!lua_toboolean(state_->L, top);
		return true;
	}
	else
		return false;
}


void Lua::PopTop()
{
	ASSERT(!IsRunning());
	lua_pop(state_->L, 1);
	PRINT_STACK_SIZE(state_->L);
}


void Lua::InitializeBuiltInLibs()
{
	if (!state_->init_libs_done_)
		state_->init_libs();
	PRINT_STACK_SIZE(state_->L);
}


void Lua::Abort(const char* msg)
{
	if (IsRunning())
		state_->abort_flag_ = true;
	else
	{
		if (msg)
			lua_pushstring(state_->L, msg);
		lua_error(state_->L);		// abort now
	}
}
