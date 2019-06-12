#ifndef __POINT_ADAPTER_EXTLIB1_H__
#define __POINT_ADAPTER_EXTLIB1_H__

#include <Eigen/Core>
#include "point_extlib1.hpp"

namespace extlib1
{

struct PointAdapter {
  public:
    enum {Dim = 3};
    typedef float Scalar;
    typedef Eigen::Matrix<Scalar, Dim, 1> VectorType;
  
  private:
    Eigen::Map<const VectorType> m_pos, m_normal, m_color;

    Scalar* underlying_data[3]  = { NULL, NULL, NULL };
    
  public:
    inline PointAdapter(const extlib1::PointType1& p)
      : m_pos   (Eigen::Map<const VectorType >( p.pos )), 
        m_normal(Eigen::Map<const VectorType >( p.n )), 
        m_color (Eigen::Map<const VectorType >( p.color ))
    { }


    inline PointAdapter()
    : underlying_data{new Scalar[Dim], new Scalar[Dim], new Scalar[Dim]},
        m_pos   (Eigen::Map<const VectorType >( underlying_data[0] )), 
        m_normal(Eigen::Map<const VectorType >( underlying_data[1] )), 
        m_color (Eigen::Map<const VectorType >( underlying_data[2] ))
    {
    }

    inline ~PointAdapter()
    {
      for(auto &ptr : underlying_data)
      {
        if(ptr) delete ptr;
        ptr = NULL;
      }
    }

    inline const Eigen::Map< const VectorType >& pos()    const { return m_pos; }  
    inline const Eigen::Map< const VectorType >& normal() const { return m_normal; }
    inline const Eigen::Map< const VectorType >& color()  const { return m_color; }
    inline const Eigen::Map< const VectorType >& rgb()    const { return m_color; }
    
};

}

#endif
