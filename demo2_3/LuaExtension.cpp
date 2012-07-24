﻿#include "stdafx.h"
#include "LuaExtension.h"
#include "Game.h"
#include "GameState.h"
#include <luabind/luabind.hpp>
#include <luabind/operator.hpp>
#include <luabind/exception_handler.hpp>

static int lua_print(lua_State * L)
{
	// ! u8tows会抛异常，不要让 C++异常直接抛到 lua函数以外
	try
	{
		MessagePanel * panel = Game::getSingleton().m_console->m_panel.get();
		_ASSERT(panel);

		int n = lua_gettop(L);  /* number of arguments */
		int i;
		lua_getglobal(L, "tostring");
		for (i=1; i<=n; i++) {
			const char *s;
			lua_pushvalue(L, -1);  /* function to be called */
			lua_pushvalue(L, i);   /* value to print */
			lua_call(L, 1, 1);
			s = lua_tostring(L, -1);  /* get result */
			if (s == NULL)
				return luaL_error(L, LUA_QL("tostring") " must return a string to "
				LUA_QL("print"));
			if (i>1) panel->puts(L"\t");
			else panel->_push_enter(D3DCOLOR_ARGB(255,255,255,255));
			panel->puts(u8tows(s));
			lua_pop(L, 1);  /* pop result */
		}
		return 0;
	}
	catch(const my::Exception & e)
	{
		return luaL_error(L, e.GetFullDescription().c_str());
	}
}

typedef struct LoadF {
	int extraline;
	//FILE *f;
	my::ArchiveStreamPtr stream;
	char buff[LUAL_BUFFERSIZE];
} LoadF;

static const char *getF (lua_State *L, void *ud, size_t *size) {
	LoadF *lf = (LoadF *)ud;
	(void)L;
	if (lf->extraline) {
		lf->extraline = 0;
		*size = 1;
		return "\n";
	}
	//if (feof(lf->f)) return NULL;
	*size = lf->stream->read(lf->buff, sizeof(lf->buff));
	return (*size > 0) ? lf->buff : NULL;
}
//
//static int errfile (lua_State *L, const char *what, int fnameindex) {
//	const char *serr = strerror(errno);
//	const char *filename = lua_tostring(L, fnameindex) + 1;
//	lua_pushfstring(L, "cannot %s %s: %s", what, filename, serr);
//	lua_remove(L, fnameindex);
//	return LUA_ERRFILE;
//}

static int luaL_loadfile (lua_State *L, const char *filename)
{
	LoadF lf;
	//int status, readstatus;
	//int c;
	int fnameindex = lua_gettop(L) + 1;  /* index of filename on the stack */
	lf.extraline = 0;
	//if (filename == NULL) {
	//	lua_pushliteral(L, "=stdin");
	//	lf.f = stdin;
	//}
	//else {
		lua_pushfstring(L, "@%s", filename);
	//	lf.f = fopen(filename, "r");
	//	if (lf.f == NULL) return errfile(L, "open", fnameindex);
	//}
	//c = getc(lf.f);
	//if (c == '#') {  /* Unix exec. file? */
	//	lf.extraline = 1;
	//	while ((c = getc(lf.f)) != EOF && c != '\n') ;  /* skip first line */
	//	if (c == '\n') c = getc(lf.f);
	//}
	//if (c == LUA_SIGNATURE[0] && filename) {  /* binary file? */
	//	lf.f = freopen(filename, "rb", lf.f);  /* reopen in binary mode */
	//	if (lf.f == NULL) return errfile(L, "reopen", fnameindex);
	//	/* skip eventual `#!...' */
	//	while ((c = getc(lf.f)) != EOF && c != LUA_SIGNATURE[0]) ;
	//	lf.extraline = 0;
	//}
	//ungetc(c, lf.f);
	try
	{
		lf.stream = my::ResourceMgr::getSingleton().OpenArchiveStream(filename);
	}
	catch(const my::Exception & e)
	{
		lua_pushfstring(L, e.GetDescription().c_str());
		lua_remove(L, fnameindex);
		return LUA_ERRFILE;
	}
	int status = lua_load(L, getF, &lf, lua_tostring(L, -1));
	//readstatus = ferror(lf.f);
	//if (filename) fclose(lf.f);  /* close file (even in case of errors) */
	//if (readstatus) {
	//	lua_settop(L, fnameindex);  /* ignore results from `lua_load' */
	//	return errfile(L, "read", fnameindex);
	//}
	lua_remove(L, fnameindex);
	return status;
}

static int load_aux (lua_State *L, int status) {
	if (status == 0)  /* OK? */
		return 1;
	else {
		lua_pushnil(L);
		lua_insert(L, -2);  /* put before error message */
		return 2;  /* return nil plus error message */
	}
}

static int luaB_loadfile (lua_State *L) {
	const char *fname = luaL_optstring(L, 1, NULL);
	return load_aux(L, luaL_loadfile(L, fname));
}

static int luaB_dofile (lua_State *L) {
	const char *fname = luaL_optstring(L, 1, NULL);
	int n = lua_gettop(L);
	if (luaL_loadfile(L, fname) != 0) lua_error(L);
	lua_call(L, 0, LUA_MULTRET);
	return lua_gettop(L) - n;
}

static int os_exit(lua_State * L)
{
	HWND hwnd = my::DxutApp::getSingleton().GetHWND();
	_ASSERT(NULL != hwnd);
	SendMessage(hwnd, WM_CLOSE, 0, 0);
	return 0;
}

static int lua_error_pcall(lua_State * L)
{
   lua_Debug d;
   lua_getstack(L, 1, &d);
   lua_getinfo(L, "Sln", &d);
   std::string err = lua_tostring(L, -1);
   lua_pop(L, 1);
   std::stringstream msg;
   msg << d.short_src << ":" << d.currentline;

   if (d.name != 0)
   {
      msg << "(" << d.namewhat << " " << d.name << ")";
   }
   msg << " " << err;
   lua_pushstring(L, msg.str().c_str());
   return 1;
}

static void translate_my_exception(lua_State* L, my::Exception const & e)
{
	std::string s = e.GetFullDescription();
	lua_pushlstring(L, s.c_str(), s.length());
}

namespace luabind
{
	template <>
	struct default_converter<std::wstring>
		: native_converter_base<std::wstring>
	{
		static int compute_score(lua_State * L, int index)
		{
			return lua_type(L, index) == LUA_TSTRING ? 0 : -1;
		}

		std::wstring from(lua_State * L, int index)
		{
			return u8tows(lua_tostring(L, index));
		}

		void to(lua_State * L, std::wstring const & str)
		{
			std::string res(wstou8(str.c_str()));
			lua_pushlstring(L, res.c_str(), res.size());
		}
	};

	template <>
	struct default_converter<std::wstring const &>
		: default_converter<std::wstring>
	{
	};

	template <>
	struct default_converter<my::ControlEvent>
		: native_converter_base<my::ControlEvent>
	{
		static int compute_score(lua_State * L, int index)
		{
			return lua_type(L, index) == LUA_TFUNCTION ? 0 : -1;
		}

		my::ControlEvent from(lua_State * L, int index)
		{
			struct InternalExceptionHandler
			{
				luabind::object obj;
				InternalExceptionHandler(const luabind::object & _obj)
					: obj(_obj)
				{
				}
				void operator()(my::EventArgsPtr args)
				{
					try
					{
						obj(args);
					}
					catch(const luabind::error & e)
					{
						Game::getSingleton().AddLine(ms2ws(lua_tostring(e.state(), -1)));
					}
				}
			};
			return InternalExceptionHandler(luabind::object(luabind::from_stack(L, index)));
		}

		void to(lua_State * L, my::ControlEvent const & e)
		{
			_ASSERT(false);
		}
	};

	template <>
	struct default_converter<my::ControlEvent const &>
		: default_converter<my::ControlEvent>
	{
	};
}

void Export2Lua(lua_State * L)
{
	lua_pushcfunction(L, lua_print);
	lua_setglobal(L, "print");

	lua_pushcfunction(L, luaB_loadfile);
	lua_setglobal(L, "loadfile");

	lua_pushcfunction(L, luaB_dofile);
	lua_setglobal(L, "dofile");

	lua_getglobal(L, "os");
	lua_pushcclosure(L, os_exit, 0);
	lua_setfield(L, -2, "exit");
	lua_pop(L, 1);

	struct HelpFunc
	{
		static DWORD ARGB(int a, int r, int g, int b)
		{
			return D3DCOLOR_ARGB(a,r,g,b);
		}
	};

	using namespace luabind;

	open(L);

	//// ! 会导致内存泄漏，但可以重写 handle_exception_aux，加入 my::Exception的支持
	//register_exception_handler<my::Exception>(&translate_my_exception);

	//// ! 为什么不起作用
	//set_pcall_callback(lua_error_pcall);

	module(L)
	[
		def("ARGB", &HelpFunc::ARGB)

		//, class_<std::wstring>("wstring")

		, class_<my::Vector2, boost::shared_ptr<my::Vector2> >("Vector2")
			.def(constructor<float, float>())
			.def_readwrite("x", &my::Vector2::x)
			.def_readwrite("y", &my::Vector2::y)
			.def(self + other<const my::Vector2 &>())
			.def(self + float())
			.def(self - other<const my::Vector2 &>())
			.def(self - float())
			.def(self * other<const my::Vector2 &>())
			.def(self * float())
			.def(self / other<const my::Vector2 &>())
			.def(self / float())
			.def("cross", &my::Vector2::cross)
			.def("dot", &my::Vector2::dot)
			.def("magnitude", &my::Vector2::magnitude)
			.def("magnitudeSq", &my::Vector2::magnitudeSq)
			.def("lerp", &my::Vector2::lerp)
			.def("lerpSelf", &my::Vector2::lerpSelf)
			.def("normalize", &my::Vector2::normalize)
			.def("normalizeSelf", &my::Vector2::normalizeSelf)
			.def("transform", &my::Vector2::transform)
			.def("transformTranspose", &my::Vector2::transformTranspose)
			.def("transformCoord", &my::Vector2::transformCoord)
			.def("transformCoordTranspose", &my::Vector2::transformCoordTranspose)
			.def("transformNormal", &my::Vector2::transformNormal)
			.def("transformNormalTranspose", &my::Vector2::transformNormalTranspose)
			.def("transform", &my::Vector2::transform)

		, class_<my::Vector3, boost::shared_ptr<my::Vector3> >("Vector3")
			.def(constructor<float, float, float>())
			.def_readwrite("x", &my::Vector3::x)
			.def_readwrite("y", &my::Vector3::y)
			.def_readwrite("z", &my::Vector3::z)
			.def(self + other<const my::Vector3 &>())
			.def(self + float())
			.def(self - other<const my::Vector3 &>())
			.def(self - float())
			.def(self * other<const my::Vector3 &>())
			.def(self * float())
			.def(self / other<const my::Vector3 &>())
			.def(self / float())
			.def("cross", &my::Vector3::cross)
			.def("dot", &my::Vector3::dot)
			.def("magnitude", &my::Vector3::magnitude)
			.def("magnitudeSq", &my::Vector3::magnitudeSq)
			.def("lerp", &my::Vector3::lerp)
			.def("lerpSelf", &my::Vector3::lerpSelf)
			.def("normalize", &my::Vector3::normalize)
			.def("normalizeSelf", &my::Vector3::normalizeSelf)
			.def("transform", (my::Vector4 (my::Vector3::*)(const my::Matrix4 &) const)&my::Vector3::transform)
			.def("transformTranspose", &my::Vector3::transformTranspose)
			.def("transformCoord", &my::Vector3::transformCoord)
			.def("transformCoordTranspose", &my::Vector3::transformCoordTranspose)
			.def("transformNormal", &my::Vector3::transformNormal)
			.def("transformNormalTranspose", &my::Vector3::transformNormalTranspose)
			.def("transform", (my::Vector3 (my::Vector3::*)(const my::Quaternion &) const)&my::Vector3::transform)

		, class_<my::Vector4, boost::shared_ptr<my::Vector4> >("Vector4")
			.def(constructor<float, float, float, float>())
			.def_readwrite("x", &my::Vector4::x)
			.def_readwrite("y", &my::Vector4::y)
			.def_readwrite("z", &my::Vector4::z)
			.def_readwrite("w", &my::Vector4::w)
			.def(self + other<const my::Vector4 &>())
			.def(self + float())
			.def(self - other<const my::Vector4 &>())
			.def(self - float())
			.def(self * other<const my::Vector4 &>())
			.def(self * float())
			.def(self / other<const my::Vector4 &>())
			.def(self / float())
			.def("cross", &my::Vector4::cross)
			.def("dot", &my::Vector4::dot)
			.def("magnitude", &my::Vector4::magnitude)
			.def("magnitudeSq", &my::Vector4::magnitudeSq)
			.def("lerp", &my::Vector4::lerp)
			.def("lerpSelf", &my::Vector4::lerpSelf)
			.def("normalize", &my::Vector4::normalize)
			.def("normalizeSelf", &my::Vector4::normalizeSelf)
			.def("transform", &my::Vector4::transform)
			.def("transformTranspose", &my::Vector4::transformTranspose)

		, class_<my::Quaternion, boost::shared_ptr<my::Quaternion> >("Quaternion")
			.def(constructor<float, float, float, float>())
			.def_readwrite("x", &my::Quaternion::x)
			.def_readwrite("y", &my::Quaternion::y)
			.def_readwrite("z", &my::Quaternion::z)
			.def_readwrite("w", &my::Quaternion::w)
			.def(self + other<const my::Quaternion &>())
			.def(self + float())
			.def(self - other<const my::Quaternion &>())
			.def(self - float())
			.def(self * other<const my::Quaternion &>())
			.def(self * float())
			.def(self / other<const my::Quaternion &>())
			.def(self / float())
			.def("conjugate", &my::Quaternion::conjugate)
			.def("conjugateSelf", &my::Quaternion::conjugateSelf)
			.def("dot", &my::Quaternion::dot)
			.def("inverse", &my::Quaternion::inverse)
			.def("magnitude", &my::Quaternion::magnitude)
			.def("magnitudeSq", &my::Quaternion::magnitudeSq)
			.def("ln", &my::Quaternion::ln)
			.def("multiply", &my::Quaternion::multiply)
			.def("normalize", &my::Quaternion::normalize)
			.def("normalizeSelf", &my::Quaternion::normalizeSelf)
			.def("lerp", &my::Quaternion::lerp)
			.def("lerpSelf", &my::Quaternion::lerpSelf)
			.def("slerp", &my::Quaternion::slerp)
			.def("slerpSelf", &my::Quaternion::slerpSelf)
			.def("squad", &my::Quaternion::squad)
			.def("squadSelf", &my::Quaternion::squadSelf)
			.scope
			[
				def("Identity", &my::Quaternion::Identity)
				, def("RotationAxis", &my::Quaternion::RotationAxis)
				, def("RotationMatrix", &my::Quaternion::RotationMatrix)
				, def("RotationYawPitchRoll", &my::Quaternion::RotationYawPitchRoll)
				, def("RotationFromTo", &my::Quaternion::RotationFromTo)
			]

		, class_<my::Matrix4, boost::shared_ptr<my::Matrix4> >("Matrix4")
			.def(self + other<const my::Matrix4 &>())
			.def(self + float())
			.def(self - other<const my::Matrix4 &>())
			.def(self - float())
			.def(self * other<const my::Matrix4 &>())
			.def(self * float())
			.def(self / other<const my::Matrix4 &>())
			.def(self / float())
			.def("inverse", &my::Matrix4::inverse)
			.def("multiply", &my::Matrix4::multiply)
			.def("multiplyTranspose", &my::Matrix4::multiplyTranspose)
			.def("transpose", &my::Matrix4::transpose)
			.def("transformTranspose", &my::Matrix4::transformTranspose)
			.def("scale", (my::Matrix4 (my::Matrix4::*)(float, float, float) const)&my::Matrix4::scale)
			.def("scale", (my::Matrix4 (my::Matrix4::*)(const my::Vector3 &) const)&my::Matrix4::scale)
			.def("scaleSelf", (my::Matrix4 & (my::Matrix4::*)(float, float, float))&my::Matrix4::scaleSelf)
			.def("scaleSelf", (my::Matrix4 & (my::Matrix4::*)(const my::Vector3 &))&my::Matrix4::scaleSelf)
			.def("rotateX", &my::Matrix4::rotateX)
			.def("rotateY", &my::Matrix4::rotateY)
			.def("rotateZ", &my::Matrix4::rotateZ)
			.def("rotate", &my::Matrix4::rotate)
			.def("translate", (my::Matrix4 (my::Matrix4::*)(float, float, float) const)&my::Matrix4::translate)
			.def("translate", (my::Matrix4 (my::Matrix4::*)(const my::Vector3 &) const)&my::Matrix4::translate)
			.def("translateSelf", (my::Matrix4 & (my::Matrix4::*)(float, float, float))&my::Matrix4::translateSelf)
			.def("translateSelf", (my::Matrix4 & (my::Matrix4::*)(const my::Vector3 &))&my::Matrix4::translateSelf)
			.def("lerp", &my::Matrix4::lerp)
			.def("lerpSelf", &my::Matrix4::lerpSelf)
			.scope
			[
				def("Identity", &my::Matrix4::Identity)
				, def("LookAtLH", &my::Matrix4::LookAtLH)
				, def("LookAtRH", &my::Matrix4::LookAtRH)
				, def("OrthoLH", &my::Matrix4::OrthoLH)
				, def("OrthoRH", &my::Matrix4::OrthoRH)
				, def("OrthoOffCenterLH", &my::Matrix4::OrthoOffCenterLH)
				, def("OrthoOffCenterRH", &my::Matrix4::OrthoOffCenterRH)
				, def("PerspectiveFovLH", &my::Matrix4::PerspectiveFovLH)
				, def("PerspectiveFovRH", &my::Matrix4::PerspectiveFovRH)
				, def("PerspectiveLH", &my::Matrix4::PerspectiveLH)
				, def("PerspectiveRH", &my::Matrix4::PerspectiveRH)
				, def("PerspectiveOffCenterLH", &my::Matrix4::PerspectiveOffCenterLH)
				, def("PerspectiveOffCenterRH", &my::Matrix4::PerspectiveOffCenterRH)
				, def("RotationAxis", &my::Matrix4::RotationAxis)
				, def("RotationQuaternion", &my::Matrix4::RotationQuaternion)
				, def("RotationX", &my::Matrix4::RotationX)
				, def("RotationY", &my::Matrix4::RotationY)
				, def("RotationZ", &my::Matrix4::RotationZ)
				, def("RotationYawPitchRoll", &my::Matrix4::RotationYawPitchRoll)
				, def("Scaling", (my::Matrix4 (*)(float, float, float))&my::Matrix4::Scaling)
				, def("Scaling", (my::Matrix4 (*)(const my::Vector3 &))&my::Matrix4::Scaling)
				, def("Transformation", &my::Matrix4::Transformation)
				, def("Transformation2D", &my::Matrix4::Transformation2D)
				, def("Translation", (my::Matrix4 (*)(float, float, float))&my::Matrix4::Translation)
				, def("Translation", (my::Matrix4 (*)(const my::Vector3 &))&my::Matrix4::Translation)
			]

		, class_<my::Spline, boost::shared_ptr<my::Spline> >("Spline")
			.def(constructor<>())
			.def("AddNode", (void (my::Spline::*)(float, float, float, float))&my::Spline::AddNode)
			.def("Interpolate", &my::Spline::Interpolate)

		, class_<my::Texture, boost::shared_ptr<my::Texture> >("Texture")

		, class_<my::Font, boost::shared_ptr<my::Font> >("Font")
			.enum_("constants")
			[
				value("AlignLeft", my::Font::AlignLeft)
				, value("AlignCenter", my::Font::AlignCenter)
				, value("AlignRight", my::Font::AlignRight)
				, value("AlignTop", my::Font::AlignTop)
				, value("AlignMiddle", my::Font::AlignMiddle)
				, value("AlignBottom", my::Font::AlignBottom)
				, value("AlignLeftTop", my::Font::AlignLeftTop)
				, value("AlignCenterTop", my::Font::AlignCenterTop)
				, value("AlignRightTop", my::Font::AlignRightTop)
				, value("AlignLeftMiddle", my::Font::AlignLeftMiddle)
				, value("AlignCenterMiddle", my::Font::AlignCenterMiddle)
				, value("AlignRightMiddle", my::Font::AlignRightMiddle)
				, value("AlignLeftBottom", my::Font::AlignLeftBottom)
				, value("AlignCenterBottom", my::Font::AlignCenterBottom)
				, value("AlignRightBottom", my::Font::AlignRightBottom)
			]

		, class_<my::EventArgs, boost::shared_ptr<my::EventArgs> >("EventArgs")

		, class_<my::ControlEvent>("ControlEvent")

		, class_<my::ControlImage, boost::shared_ptr<my::ControlImage> >("ControlImage")
			.def(constructor<boost::shared_ptr<my::Texture>, const my::Vector4 &>())

		, class_<my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("ControlSkin")
			.def(constructor<>())
			.def_readwrite("Image", &my::ControlSkin::m_Image)
			.def_readwrite("Font", &my::ControlSkin::m_Font)
			.def_readwrite("TextColor", &my::ControlSkin::m_TextColor)
			.def_readwrite("TextAlign", &my::ControlSkin::m_TextAlign)

		, class_<my::Control, boost::shared_ptr<my::Control> >("Control")
			.def(constructor<>())
			.property("Enabled", &my::Control::GetEnabled, &my::Control::SetEnabled)
			.property("Visible", &my::Control::GetVisible, &my::Control::SetVisible)
			.def_readwrite("Location", &my::Control::m_Location)
			.def_readwrite("Size", &my::Control::m_Size)
			.def_readwrite("Color", &my::Control::m_Color)
			.def_readwrite("Skin", &my::Control::m_Skin)

		, class_<my::Static, my::Control, boost::shared_ptr<my::Control> >("Static")
			.def(constructor<>())
			.def_readwrite("Text", &my::Static::m_Text)

		, class_<my::ButtonSkin, my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("ButtonSkin")
			.def(constructor<>())
			.def_readwrite("DisabledImage", &my::ButtonSkin::m_DisabledImage)
			.def_readwrite("PressedImage", &my::ButtonSkin::m_PressedImage)
			.def_readwrite("MouseOverImage", &my::ButtonSkin::m_MouseOverImage)

		, class_<my::Button, my::Static, boost::shared_ptr<my::Control> >("Button")
			.def(constructor<>())
			.def_readwrite("PressedOffset", &my::Button::m_PressedOffset)
			.def_readwrite("EventClick", &my::Button::EventClick)
			.def("SetHotkey", &my::Button::SetHotkey)

		, class_<my::EditBoxSkin, my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("EditBoxSkin")
			.def(constructor<>())
			.def_readwrite("DisabledImage", &my::EditBoxSkin::m_DisabledImage)
			.def_readwrite("FocusedImage", &my::EditBoxSkin::m_FocusedImage)
			.def_readwrite("SelTextColor", &my::EditBoxSkin::m_SelTextColor)
			.def_readwrite("SelBkColor", &my::EditBoxSkin::m_SelBkColor)
			.def_readwrite("CaretColor", &my::EditBoxSkin::m_CaretColor)

		, class_<my::EditBox, my::Static, boost::shared_ptr<my::Control> >("EditBox")
			.def(constructor<>())
			.property("Text", &my::EditBox::GetText, &my::EditBox::SetText)
			.def_readwrite("Border", &my::EditBox::m_Border)
			.def_readwrite("EventChange", &my::EditBox::EventChange)
			.def_readwrite("EventEnter", &my::EditBox::EventEnter)

		, class_<my::ImeEditBox, my::EditBox, boost::shared_ptr<my::Control> >("ImeEditBox")
			.def(constructor<>())

		, class_<my::ScrollBarSkin, my::ControlSkin, boost::shared_ptr<my::ControlSkin> >("ScrollBarSkin")
			.def(constructor<>())
			.def_readwrite("UpBtnNormalImage", &my::ScrollBarSkin::m_UpBtnNormalImage)
			.def_readwrite("UpBtnDisabledImage", &my::ScrollBarSkin::m_UpBtnDisabledImage)
			.def_readwrite("DownBtnNormalImage", &my::ScrollBarSkin::m_DownBtnNormalImage)
			.def_readwrite("DownBtnDisabledImage", &my::ScrollBarSkin::m_DownBtnDisabledImage)
			.def_readwrite("ThumbBtnNormalImage", &my::ScrollBarSkin::m_ThumbBtnNormalImage)

		, class_<my::ScrollBar, my::Control, boost::shared_ptr<my::Control> >("ScrollBar")
			.def(constructor<>())
			.def_readwrite("nPosition", &my::ScrollBar::m_nPosition) // ! should be removed
			.def_readwrite("nPageSize", &my::ScrollBar::m_nPageSize) // ! should be removed
			.def_readwrite("nStart", &my::ScrollBar::m_nStart) // ! should be removed
			.def_readwrite("nEnd", &my::ScrollBar::m_nEnd) // ! should be removed

		, class_<my::AlignEventArgs, my::EventArgs, boost::shared_ptr<my::EventArgs> >("AlignEventArgs")
			.def_readonly("vp", &my::AlignEventArgs::vp)

		, class_<my::Dialog, my::Control, boost::shared_ptr<my::Dialog> >("Dialog")
			.def(constructor<>())
			.def_readwrite("Transform", &my::Dialog::m_Transform)
			.def_readwrite("EventAlign", &my::Dialog::EventAlign)
			.def("InsertControl", &my::Dialog::InsertControl)

		//, class_<MessagePanel, my::Control, boost::shared_ptr<my::Control> >("MessagePanel")
		//	.def(constructor<>())
		//	.def_readwrite("lbegin", &MessagePanel::m_lbegin)
		//	.def_readwrite("lend", &MessagePanel::m_lend)
		//	.def_readwrite("scrollbar", &MessagePanel::m_scrollbar)
		//	.def("AddLine", &MessagePanel::AddLine)
		//	.def("puts", &MessagePanel::puts)

		//, class_<ConsoleEditBox, my::ImeEditBox, boost::shared_ptr<my::Control> >("ConsoleEditBox")
		//	.def(constructor<>())
		//	.def_readwrite("EventKeyUp", &ConsoleEditBox::EventKeyUp)
		//	.def_readwrite("EventKeyDown", &ConsoleEditBox::EventKeyDown)

		, class_<Game>("Game")
			.def_readwrite("font", &Game::m_font)
			.def("CurrentState", &Game::CurrentState)
			.def("process_event", &Game::process_event)
			.scope
			[
				def("LoadTexture", &Game::LoadTexture)
				, def("LoadFont", &Game::LoadFont)
			]
			.def("ToggleFullScreen", &Game::ToggleFullScreen)
			.def("ToggleRef", &Game::ToggleRef)
			.def("ChangeDevice", &Game::ChangeDevice)
			.def("ExecuteCode", &Game::ExecuteCode)
			.def("InsertDlg", &Game::InsertDlg)

		, class_<GameStateBase>("GameStateBase")

		, class_<GameEventBase, boost::shared_ptr<GameEventBase> >("GameEventBase")

		, class_<GameEventLoadOver, GameEventBase, boost::shared_ptr<GameEventBase> >("GameEventLoadOver")
			.def(constructor<>())

		, class_<GameStateLoad, GameStateBase>("GameStateLoad")

		, class_<GameStatePlay, GameStateBase>("GameStatePlay")

		//, class_<BaseCamera, boost::shared_ptr<BaseCamera> >("BaseCamera")
		//	.def_readwrite("Aspect", &BaseCamera::m_Aspect)
		//	.def_readwrite("Nz", &BaseCamera::m_Nz)
		//	.def_readwrite("Fz", &BaseCamera::m_Fz)

		//, class_<Camera, BaseCamera, boost::shared_ptr<BaseCamera> >("Camera")
		//	.def(constructor<float, float, float, float>())
		//	.def_readwrite("Position", &Camera::m_Position)
		//	.def_readwrite("Orientation", &Camera::m_Orientation)
		//	.def_readwrite("Fovy", &Camera::m_Fovy)

		//, class_<ModuleViewCamera, BaseCamera, boost::shared_ptr<BaseCamera> >("ModuleViewCamera")
		//	.def(constructor<float, float, float, float>())
		//	.def_readwrite("LookAt", &ModuleViewCamera::m_LookAt)
		//	.def_readwrite("Rotation", &ModuleViewCamera::m_Rotation)
		//	.def_readwrite("Fovy", &ModuleViewCamera::m_Fovy)
		//	.def_readwrite("Distance", &ModuleViewCamera::m_Distance)

		//, class_<BaseScene, boost::shared_ptr<BaseScene> >("BaseScene")

		//, class_<Scene, BaseScene, boost::shared_ptr<BaseScene> >("Scene")
		//	.def(constructor<>())
		//	.def_readwrite("Camera", &Scene::m_Camera)
	];

	globals(L)["game"] = Game::getSingletonPtr();
}
