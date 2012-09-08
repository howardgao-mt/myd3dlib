function SetupMaterial(material)
	material.Effect = LoadEffect("Character.fx")
	material:BeginParameterBlock("RenderScene")
	material.Effect:SetVector("g_MaterialAmbientColor", Vector4(0.3,0.3,0.3,1.0))
	material.Effect:SetVector("g_MaterialDiffuseColor", Vector4(1.0,1.0,1.0,1.0))
	material.Effect:SetTexture("g_MeshTexture", LoadTexture("casual19_m_35.jpg"))
	material.Effect:SetTexture("g_NormalTexture", LoadTexture("casual19_m_35_normal.png"))
	material.Effect:SetTexture("g_SpecularTexture", LoadTexture("casual19_m_35_spec.png"))
	material.Effect:SetTexture("g_CubeTexture", LoadCubeTexture("galileo_cross.dds"))
	material:EndParameterBlock()
end