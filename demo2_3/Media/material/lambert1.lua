local material=Material()
material.Effect = game:LoadEffect("shader/SimpleSample.fx")
material.ParameterMap:SetVector("g_MaterialAmbientColor", Vector4(0.3,0.3,0.3,1.0))
material.ParameterMap:SetVector("g_MaterialDiffuseColor", Vector4(1.0,1.0,1.0,1.0))
material.ParameterMap:SetTexture("g_MeshTexture", game:LoadTexture("texture/Checker.bmp"))
game:InsertMaterial("lambert1", material)