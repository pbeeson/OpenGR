# Using Your Point Type

OpenGR provides gr::PointConcept, which it relies on while operating with the point samples internally. While gr::PointConcept yields the least requirements for any point type that could be used as input to OpenGR, additional features could be utilised to employ different point filters while comparing two point instances considering those additional features.

When an existing external point type is going to be used as input to OpenGR, gr::PointConcept can be implemented to behave as a wrapper (i.e. adapter) for the external point type. In this case, two types will be in question: the external point type whose instances are available to be passed to OpenGR, and the point type whose implementation is available to work as an adapter of the external point type to provide required interface to OpenGR. For registration, at the sampling stage, points are reinstantiated of the point adapter type which is given as template point type parameter. The requirement with gr::PointConcept for a constructor, which takes an instance of the external point type as input, is handy when the given point adapter type is not the same as the type of the input point instances. In other words, this constructor is used to reinstantiate the point sample with the given point adapter type at the sampling stage, so that, the provided interface could be used internally through the adapter.

In the other case, if the type of the input point instances provides the required interface, the type of input point instances could be passed as the template type parameter. This way, the required constructor turns out to be the copy constructor of the point type. Therefore, the sample points are copied using the copy constructor of the point type at the sampling stage. Although this allows to avoid implementing an additional adapter type, one could consider using an mapper adapter if the copy constructor causes memory duplicates of the underlying point sample data.

Internally, OpenGR uses Eigen for computation, vector representation and vectorization. Therefore, it expects PointConcept::VectorType to be compatible with Eigen::DenseBase, as apparent in PointConcept. To use existing point types without requiring any memory duplication while complying with this requirement, Eigen::Map could be used to map data of existing point types as an Eigen matrix or vector.

Following example shows a simple point type class demonstrating a possible point type external to OpenGR, and an adapter for the point type that allows OpenGR to work on the instances of this point type, without needing any duplication of data while complying with the required interface of the point type concept: gr::PointConcept.

```{.cpp}
#include <Eigen/Core>

namespace extlib
{
  // Some external point type
  struct PointType {
    float* posBuffer;   // position buffer
    float* nBuffer;     // normal buffer
    float* colorBuffer; // color buffer
    int id;             // id (or index)
  };
}

// Adapter for extlib::PointType, implements gr::PointConcept
struct PointAdapter {
  public:
    // Required
    enum {Dim = 3};

    // Required
    typedef float Scalar;

    // Required
    typedef Eigen::Matrix<Scalar, Dim, 1> VectorType;
  
  private:
    // Hold mapping information instead of copying underlying data 
    // to avoid unnecessary memory duplicates.
    Eigen::Map< const VectorType > m_pos, m_normal, m_color;
  
  public:
    // Required
    inline PointAdapter(const extlib::PointType& p)
      : m_pos   (Eigen::Map< const VectorType >( p.posBuffer + Dim*p.id )), 
        m_normal(Eigen::Map< const VectorType >( p.nBuffer + Dim*p.id )), 
        m_color (Eigen::Map< const VectorType >( p.colorBuffer + Dim*p.id ))
    { }

    // Required
    inline const Eigen::Map< const VectorType >& pos()    const { return m_pos; }  

    // Optional: could be used by point filters that require such attributes
    inline const Eigen::Map< const VectorType >& normal() const { return m_normal; }
    inline const Eigen::Map< const VectorType >& color()  const { return m_color; }
};
```

As apperent in above example, to use your own point type, it is sufficient to implement a point adapter, and pass the type of the point adapter as the point type while instantiating any template class or method. OpenGR will wrap your point type with the adapter, and use the interface provided by the adapter for its computations.

For convenience, OpenGR provides an implementation for gr::PointConcept: gr::Point3D. IO methods for gr::Point3D is also provided with the library to use the library on supported point cloud files without needing to implement the point concept and the IO methods. 