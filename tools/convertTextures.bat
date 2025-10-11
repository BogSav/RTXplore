.\texassemble.exe array -o Terrain.Array.Normal.dds RiverRocks.Normal.jpg Terrain.Normal.jpg Rocks.Normal.jpg Snow.Normal.jpg
.\texconv -pow2 -y Terrain.Array.Normal.dds

.\texassemble.exe array -o Terrain.Array.Diffuse.dds RiverRocks.Diffuse.jpg Terrain.Diffuse.jpg Rocks.Diffuse.jpg Snow.Diffuse.jpg
.\texconv -pow2 -y Terrain.Array.Diffuse.dds

.\texassemble.exe array -o Terrain.Array.Disp.dds RiverRocks.Disp.jpg Terrain.Disp.jpg Rocks.Disp.jpg Snow.Disp.jpg
.\texconv -pow2 -y Terrain.Array.Disp.dds

