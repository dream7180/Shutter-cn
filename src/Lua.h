/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <exception>
#include <vector>


typedef std::vector<unsigned char> ProgBuf;


struct lua_exception : public std::exception
{
	lua_exception(const char* msg) : exception(msg)
	{}
};


namespace lua_details
{
	struct State;
}


class Lua : boost::noncopyable
{
public:
	Lua();
	~Lua();

	enum Event { Start, Running, NewLine, Finished };

	void SetCallback(const boost::function<void (Event, int)>& fn);

	// load file (parse, do not execute)
	void LoadFile(const char* file_path);
	bool LoadFile(const char* file_path, const char*& msg);

	// 'load' string
	bool LoadBuffer(const char* source, const char*& msg, const char* name= 0);

	// dump current program into the buffer (as executable byte codes)
	void Dump(ProgBuf& program, bool debug);

	// for synchronous execution: initialize libs
	void InitializeBuiltInLibs();

	// execute (synchronous/blocking execution: on this/calling thread)
	int Call(int num_arg, const char** msg= 0);
	int Call(int num_arg, int max_steps_to_execute, const char** msg);

	// execute single line (current one) following calls, if any
	void StepInto();
	// execute current line, without entering any functions
	void StepOver();
	// start execution (it runs in a thread)
	void Run();
	// run till return from the current function
	void StepOut();
	// execute up to max_steps (lines) and stop
	void ExecuteNSteps(int max_steps);
	// abort execution (with optional message for synchronously running program, not a threaded one)
	void Abort(const char* msg);

	std::string Status() const;

	bool IsRunning() const;		// is Lua program running now? (if not, maybe it stopped at the breakpoint)
	bool IsFinished() const;	// has Lua program finished execution?
	bool IsStopped() const;		// if stopped, it can be resumed (if not stopped, it's either running or done)

	// toggle breakpoint in given line
	bool ToggleBreakpoint(int line);

	// stop running program
	void Break();

	// strict global variables access rules
	void SetStrict();

	// manipulation functions
	void NewTable();
	void SetMetaTable();
	void GetMetaTable();
	void SetField(const char* key, double value);
	void SetField(const char* key, const char* value);
	void SetField(const char* key, const boost::function<int (Lua* self)>& fn);
	void SetFieldUser(const char* key, void* user_data);
	void SetField(const char* key);	// use value at the top of the stack
	void GetField(const char* key, int tbl_idx= -1);
	void* GetUserData(int idx= -1);
	void SetGlobal(const char* name);
	void GetGlobal(const char* name);
	std::string GetTop() const;
	const char* StackPeek(int index, bool translate) const;
	bool GetTop(double& val) const;
	bool GetTop(bool& val) const;
	void PopTop();
	void PushNil();
	void PushValue(int n);
#ifdef _WIN64
	void PushValue(unsigned int n);
#endif
	void PushValue(size_t n);
	void PushValue(double d);
	void PushValue(const char* str);
	void PushValue(const std::string& str);
	void PushValue(bool b);
	void PushUserValue(void* user_data);

	// get current call stack
	std::string GetCallStack() const;

	enum ValType { None= -1, Nil, Bool, LightUserData, Number, String, Table, Function, UserData, Thread };

	struct Value	// value on a virtual stack or elsewhere
	{
		Value() : type(None), type_name(0)
		{}

		ValType type;
		const char* type_name;
		std::string value;	// simplified string representation of value
	};

	struct Var
	{
		std::string name;	// variable's identifier
		Value v;
	};

	// get local vars of function at given 'level'
	bool GetLocalVars(std::vector<Var>& out, int level= 0) const;

	struct Field	// table entry
	{
		Value key;
		Value val;
	};

	typedef std::vector<Field> TableInfo;

	// get global vars
//	bool GetGlobalVars(TableInfo& out, bool deep) const;

	typedef std::vector<Value> ValueStack;

	// read all values off virtual value stack
	bool GetValueStack(ValueStack& stack) const;

	struct StackFrame
	{
		StackFrame();
		void Clear();
		const char* SourcePath() const;

		enum Entry { Err, LuaFun, MainChunk, CFun, TailCall } type;
		std::string source;
		std::string name_what;
		int current_line;	// 1..N or 0 if not available
		// where it is defined (Lua fn)
		int line_defined;
		int last_line_defined;
	};
	// 
	typedef std::vector<StackFrame> CallStack;
	
	// get function call stack
	bool GetCallStack(CallStack& stack) const;

	// info about current function and source file (at the top of the stack)
	bool GetCurrentSource(StackFrame& top) const;

private:
	boost::scoped_ptr<lua_details::State> state_;
};
