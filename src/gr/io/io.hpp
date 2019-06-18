//
// Created by Necip Fazil Yildiran on 06/18/19.
//

#include "gr/io/io.h"

#include <string>
#include <iterator>

template<typename PointRange,
         typename TextCoordRange,
         typename NormalRange,
         typename TrisRange,
         typename MTLSRange>
bool IOManager::WriteObject(
  const char *name,
  const PointRange &v,
  const TextCoordRange &tex_coords,
  const NormalRange &normals,
  const TrisRange &tris,
  const MTLSRange &mtls)
{
  std::string filename (name);
  std::string ext = filename.substr(filename.size()-3);

  bool haveExt = filename.at(filename.size()-4) == '.';

  if (tris.size() == 0){
    return WritePly(haveExt ?
                    filename.substr(0,filename.size()-3).append("ply") :
                    filename.append(".ply"),
                    v, normals);
  }
  else{
    return WriteObj(haveExt ?
                    filename.substr(0,filename.size()-3).append("obj") :
                    filename.append(".obj"),
                    v,
                    tex_coords,
                    normals,
                    tris,
                    mtls);
  }
}

template<typename PointRange, typename NormalRange>
bool
IOManager::WritePly(
  std::string filename,
  const PointRange &v,
  const NormalRange &normals)
{
  std::ofstream plyFile;
  plyFile.open (filename.c_str(), std::ios::out |  std::ios::trunc | std::ios::binary);
  if (! plyFile.is_open()){
    std::cerr << "Cannot open file to write!" << std::endl;
    return false;
  }

  bool useNormals = normals.size() == v.size();
  // we check if we have colors by looking if the first rgb vector is void
  auto has_color = [](const typename PointRange::value_type& p ) { 
    using Scalar = typename PointRange::value_type::Scalar;
    return p.rgb().squaredNorm() > Scalar(0.001);
  };
  bool useColors = false;
  for(const auto& p : v)
  {
    if( has_color(p) )
    {
      useColors = true;
      break;
    }
  }

  plyFile.imbue(std::locale::classic());

  // Write Header
  plyFile << "ply" << std::endl;
  plyFile << "format binary_little_endian 1.0" << std::endl;
  plyFile << "comment Super4PCS output file" << std::endl;
  plyFile << "element vertex " << v.size() << std::endl;
  plyFile << "property float x" << std::endl;
  plyFile << "property float y" << std::endl;
  plyFile << "property float z" << std::endl;

  if(useNormals) {
    plyFile << "property float nx" << std::endl;
    plyFile << "property float ny" << std::endl;
    plyFile << "property float nz" << std::endl;
  }

  if(useColors) {
    plyFile << "property uchar red" << std::endl;
    plyFile << "property uchar green" << std::endl;
    plyFile << "property uchar blue" << std::endl;
  }

  plyFile << "end_header" << std::endl;

  // Read all elements in data, correct their depth and print them in the file
  char tmpChar;
  float tmpFloat;
  typename NormalRange::const_iterator normal_it = normals.cbegin();
  for(const auto& p : v)
  {
    tmpFloat = static_cast<float>(p.pos()(0));
    plyFile.write(reinterpret_cast<const char*>(&tmpFloat),sizeof(float));
    tmpFloat = static_cast<float>(p.pos()(1));
    plyFile.write(reinterpret_cast<const char*>(&tmpFloat),sizeof(float));
    tmpFloat = static_cast<float>(p.pos()(2));
    plyFile.write(reinterpret_cast<const char*>(&tmpFloat),sizeof(float));

    if(useNormals) // size check is done earlier
    {
      plyFile.write(reinterpret_cast<const char*>( &(*normal_it)(0) ),sizeof(float));
      plyFile.write(reinterpret_cast<const char*>( &(*normal_it)(1) ),sizeof(float));
      plyFile.write(reinterpret_cast<const char*>( &(*normal_it)(2) ),sizeof(float));
      normal_it++;
    }

    if(useColors)
    {
      tmpChar = static_cast<char>( p.rgb()(0) );
      plyFile.write(reinterpret_cast<const char*>(&tmpChar),sizeof(char));
      tmpChar = static_cast<char>( p.rgb()(1) );
      plyFile.write(reinterpret_cast<const char*>(&tmpChar),sizeof(char));
      tmpChar = static_cast<char>( p.rgb()(2) );
      plyFile.write(reinterpret_cast<const char*>(&tmpChar),sizeof(char));

    }
  }

  plyFile.close();

  return true;
}

template<typename PointRange,
         typename TexCoordRange,
         typename NormalRange,
         typename TrisRange,
         typename MTLSRange>
bool
IOManager::WriteObj(std::string filename,
         const PointRange &v,
         const TexCoordRange &tex_coords,
         const NormalRange &normals,
         const TrisRange &tris,
         const MTLSRange &mtls)
{
  std::fstream f(filename.c_str(), std::ios::out);
  if (!f || f.fail()) return false;
  size_t i;

  for(const auto& m : mtls)
  {
    f << "mtllib " << m << std::endl;
  }

  for(const auto& p : v)
  {
    f << "v "
      << p.pos()(0) << " " << p.pos()(1) << " " << p.pos()(2) << " ";

    if (p.rgb()(0) != 0) // TODO: What about hasColor?
      f << p.rgb()(0) << " " << p.rgb()(1) << " " << p.rgb()(2);

    f << std::endl;
  }

  for(const auto& n : normals)
  {
    f << "vn " << n(0) << " " << n(1) << " " << n(2)
      << std::endl;
  }

  for(const auto& t : tex_coords)
  {
    f << "vt " << t(0) << " " << t(1) << std::endl;
  }

  auto is_normals_empty = [&]() { return normals.begin() == normals.end(); };
  auto is_texcoords_empty = [&]() { return tex_coords.begin() == tex_coords.end(); };

  for(const auto& t : tris)
  {
    if(is_normals_empty() && is_texcoords_empty())
      f << "f " << t.a << " " << t.b << " " << t.c << std::endl;
    else if(!is_texcoords_empty())
      f << "f " << t.a << "/" << t.t1 << " " << t.b << "/"
        << t.t2 << " " << t.c << "/" << t.t3 << std::endl;
    else
      f << "f " << t.a << "/" << t.n1 << " " << t.b << "/"
        << t.n2 << " " << t.c << "/" << t.n3 << std::endl;
  }

  f.close();

  return true;
}