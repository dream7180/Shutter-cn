/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/


template <class _Tp>
class mem_fun_ref_v_t : public unary_function<_Tp*,void>
{
  typedef void (_Tp::*_fun_type)(void);
public:
  explicit mem_fun_ref_v_t(_fun_type __pf) : _M_f(__pf) {}
  void operator()(_Tp* __p) const { (__p->*_M_f)(); }
private:
  _fun_type _M_f;
};

template <class _Tp>
inline mem_fun_ref_v_t<_Tp> mem_fun_ref_v(void (_Tp::*__f)()) { return mem_fun_ref_v_t<_Tp>(__f); }


template <class _Tp, class _Arg>
class mem_fun1_v_t : public binary_function<_Tp*,_Arg,void> {
  typedef void (_Tp::*_fun_type)(_Arg);
public:
  explicit mem_fun1_v_t(_fun_type __pf) : _M_f(__pf) {}
  void operator()(_Tp* __p, _Arg __x) const { (__p->*_M_f)(__x); }
private:
  _fun_type _M_f;
};

template <class _Tp, class _Arg>
class mem_fun1_ref_v_t : public binary_function<_Tp,_Arg,void> {
  typedef void (_Tp::*_fun_type)(_Arg);
public:
  explicit mem_fun1_ref_v_t(_fun_type __pf) : _M_f(__pf) {}
  void operator()(_Tp& __r, _Arg __x) const { (__r.*_M_f)(__x); }
private:
  _fun_type _M_f;
};


template <class _Tp, class _Arg>
inline mem_fun1_ref_v_t<_Tp,_Arg> mem_fun_ref_v(void (_Tp::*__f)(_Arg)) { return mem_fun1_ref_v_t<_Tp,_Arg>(__f); }


template <class _T>
class mem_fun_v_t : public unary_function<_T*,void>
{
  typedef void (_T::*_fun_type)(void);
public:
  explicit mem_fun_v_t(_fun_type __pf) : _M_f(__pf) {}
  void operator()(_T& __t) const { (__t.*_M_f)(); }
private:
  _fun_type _M_f;
};

template <class _T>
inline mem_fun_v_t<_T> mem_fun_v(void (_T::*__f)()) { return mem_fun_v_t<_T>(__f); }
