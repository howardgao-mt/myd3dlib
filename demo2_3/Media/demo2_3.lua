require "Hud.lua"
require "Settings.lua"

-- 设置Device Settings事件
Settings.dlg.Visible=false
Hud.btn_change_device.EventClick=function(args)
	Settings.dlg.Visible=not Settings.dlg.Visible
end

-- 获取当前state
local state=game:CurrentState()

-- -- 创建相机
-- local camera=ModelViewerCamera(math.rad(75),4/3.0,0.1,3000)
-- camera.Rotation=Vector3(math.rad(45),math.rad(-45),0)
-- camera.LookAt=Vector3(0,1,0)
-- camera.Distance=3
-- state.Camera=camera

local camera=FirstPersonCamera(math.rad(75),4/3.0,0.1,3000)
local k=math.cos(math.rad(45))
local d=3
camera.Position=Vector3(d*k*k,d*k+1,-d*k*k)
camera.Rotation=Vector3(math.rad(45),math.rad(-45),0)
state.Camera=camera

-- ! 利用EventAlign调整相机的Aspect
local d=Dialog();d.Visible=false;d.EventAlign=function(args) camera.Aspect=args.vp.x/args.vp.y end;game:InsertDlg(d)

-- 创建场景
local function CreateScene(n)
	state:InsertStaticMesh(Loader.LoadEffectMesh(n..".mesh.xml"))
end

-- 创建角色
local function CreateRole(n,p,t)
	local character=Character()
	character:InsertMeshLOD(Loader.LoadEffectMesh(n..".mesh.xml"))
	character:InsertSkeletonLOD(Loader.LoadSkeleton(n..".skeleton.xml"))
	character.Scale=Vector3(0.01,0.01,0.01)
	character.Position=p
	character.StateTime=t
	state:InsertCharacter(character)
end

-- CreateScene("plane")

CreateScene("water")

CreateRole("casual19_m_highpoly", Vector3(0,0,0), 0)

-- for i=-5,5 do
	-- for j=-5,5 do
		-- CreateRole("casual19_m_highpoly", Vector3(i,0,j), math.random(0,1))
	-- end
-- end