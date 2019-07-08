#include "gr/io/io.h"
#include "gr/utils/geometry.h"
#include "gr/sampling.h"
#include "gr/algorithms/match4pcsBase.h"
#include "gr/algorithms/Functor4pcs.h"
#include "gr/algorithms/FunctorSuper4pcs.h"
#include "gr/algorithms/FunctorBrute4pcs.h"
#include <gr/algorithms/PointPairFilter.h>

#include <Eigen/Dense>

#include <fstream>
#include <iostream>
#include <string>

#include "../demo-utils.h"

#include "ext/point_conversion.h"
#include "ext/point_extlib1.hpp"
#include "ext/point_extlib2.hpp"
#include "ext/pointadapter_extlib1.hpp"
#include "ext/pointadapter_extlib2.hpp"

#define sqr(x) ((x) * (x))

using namespace std;
using namespace gr;
using namespace gr::Demo;



// data IO
IOManager iomananger;


static inline void printS4PCSParameterList(){
    fprintf(stderr, "\t[ -r result_file_name (%s) ]\n", output.c_str());
    fprintf(stderr, "\t[ -m output matrix file (%s) ]\n", outputMat.c_str());
    fprintf(stderr, "\t[ -x (use 4pcs: false by default) ]\n");
    fprintf(stderr, "\t[ --sampled1 (output sampled cloud 1 -- debug+super4pcs only) ]\n");
    fprintf(stderr, "\t[ --sampled2 (output sampled cloud 2 -- debug+super4pcs only) ]\n");
}
struct TransformVisitor {
    template <typename Derived>
    inline void operator()(
            float fraction,
            float best_LCP,
            const Eigen::MatrixBase<Derived>& /*transformation*/) const {
      if (fraction >= 0)
        {
          printf("done: %d%c best: %f                  \r",
               static_cast<int>(fraction * 100), '%', best_LCP);
          fflush(stdout);
        }
    }
    constexpr bool needsGlobalTransformation() const { return false; }
};

template <
    typename Matcher,
    typename PointType,
    typename Options,
    typename Range,
    typename Sampler,
    typename TransformVisitor>
typename PointType::Scalar computeAlignment (
    const Options& options,
    const Utils::Logger& logger,
    const Range& P,
    const Range& Q,
    Eigen::Ref<Eigen::Matrix<typename PointType::Scalar, 4, 4>> mat,
    const Sampler& sampler,
    TransformVisitor& visitor
    ) {
  Matcher matcher (options, logger);
  logger.Log<Utils::Verbose>( "Starting registration" );
  typename PointType::Scalar score = matcher.ComputeTransformation(P, Q, mat, sampler, visitor);


  logger.Log<Utils::Verbose>( "Score: ", score );
  logger.Log<Utils::Verbose>( "(Homogeneous) Transformation from ",
                              input2.c_str(),
                              " to ",
                              input1.c_str(),
                              ": \n",
                              mat);

  if(! outputSampled1.empty() ){
      logger.Log<Utils::Verbose>( "Exporting Sampled cloud 1 to ",
                                  outputSampled1.c_str(),
                                  " ..." );
      iomananger.WriteObject((char *)outputSampled1.c_str(),
                             matcher.getFirstSampled(),
                             vector<Eigen::Matrix2f>(),
                             vector<typename Point3D<float>::VectorType>(), // dummy
                             vector<tripple>(),
                             vector<string>());
      logger.Log<Utils::Verbose>( "Export DONE" );
  }
  if(! outputSampled2.empty() ){
      logger.Log<Utils::Verbose>( "Exporting Sampled cloud 2 to ",
                                  outputSampled2.c_str(),
                                  " ..." );
      iomananger.WriteObject((char *)outputSampled2.c_str(),
                             matcher.getSecondSampled(),
                             vector<Eigen::Matrix2f>(),
                             vector<typename Point3D<float>::VectorType>(), // dummy
                             vector<tripple>(),
                             vector<string>());
      logger.Log<Utils::Verbose>( "Export DONE" );
  }

  return score;
}

int main(int argc, char **argv) {
  using namespace gr;
  using Scalar = float;
  // Point clouds are read as gr::Point3D, then converted to other types if necessary to
  // emulate PointAdapter usage
  vector<Point3D<Scalar> > set1, set2;
  vector<Eigen::Matrix2f> tex_coords1, tex_coords2;
  vector<typename Point3D<Scalar>::VectorType> normals1, normals2;
  vector<tripple> tris1, tris2;
  vector<std::string> mtls1, mtls2;

  // Match and return the score (estimated overlap or the LCP).
  typename Point3D<Scalar>::Scalar score = 0;

  constexpr Utils::LogLevel loglvl = Utils::Verbose;
  using SamplerType   = gr::UniformDistSampler;
  using TrVisitorType = typename std::conditional <loglvl==Utils::NoLog,
                            DummyTransformVisitor,
                            TransformVisitor>::type;
  using PairFilter = gr::AdaptivePointFilter;

  SamplerType sampler;
  TrVisitorType visitor;
  Utils::Logger logger(loglvl);

  /// TODO Add proper error codes
  if(argc < 4){
      Demo::printUsage(argc, argv);
      exit(-2);
  }
  if(int c = Demo::getArgs(argc, argv) != 0)
  {
    Demo::printUsage(argc, argv);
    printS4PCSParameterList();
    exit(std::max(c,0));
  }

  // prepare matcher ressourcesoutputSampled2
  using MatrixType = Eigen::Matrix<typename Point3D<Scalar>::Scalar, 4, 4>;
  MatrixType mat (MatrixType::Identity());

  // Read the inputs.
  if (!iomananger.ReadObject((char *)input1.c_str(), set1, tex_coords1, normals1, tris1,
                  mtls1)) {
    logger.Log<Utils::ErrorReport>("Can't read input set1");
    exit(-1);
  }

  if (!iomananger.ReadObject((char *)input2.c_str(), set2, tex_coords2, normals2, tris2,
                  mtls2)) {
    logger.Log<Utils::ErrorReport>("Can't read input set2");
    exit(-1);
  }

  // clean only when we have pset to avoid wrong face to point indexation
  if (tris1.size() == 0)
    Utils::CleanInvalidNormals(set1, normals1);
  if (tris2.size() == 0)
    Utils::CleanInvalidNormals(set2, normals2);

  try {

      if (use_super4pcs) {
        if(point_type == 0) // gr::Point3D, directly pass the read sets
        {
          using PointType    = gr::Point3D<Scalar>;
          using PointAdapter = gr::Point3D<Scalar>;
          using MatcherType  = gr::Match4pcsBase<gr::FunctorSuper4PCS, PointAdapter, TrVisitorType, gr::AdaptivePointFilter, gr::AdaptivePointFilter::Options>;
          using OptionType   = typename MatcherType::OptionsType;

          OptionType options;
          if(! Demo::setOptionsFromArgs(options, logger))
            exit(-2); /// \FIXME use status codes for error reporting

          score = computeAlignment<MatcherType, PointAdapter> (options, logger, set1, set2, mat, sampler, visitor);
        }
        else if(point_type == 1) // extlib1::PointType1, convert read sets to vector of PointType1 to emulate PointAdapter usage
        {
          logger.Log<Utils::Verbose>("Registration on std::vector of extlib1::PointType1 instances using extlib1::PointAdapter");
          using PointType    = extlib1::PointType1;
          using PointAdapter = extlib1::PointAdapter;
          using MatcherType = gr::Match4pcsBase<gr::FunctorSuper4PCS, PointAdapter, TrVisitorType, gr::AdaptivePointFilter, gr::AdaptivePointFilter::Options>;
          using OptionType  = typename MatcherType::OptionsType;

          OptionType options;
          if(! Demo::setOptionsFromArgs(options, logger))
            exit(-2); /// \FIXME use status codes for error reporting

          auto set1_point1 = getExtlib1Points(set1);
          auto set2_point1 = getExtlib1Points(set2);

          score = computeAlignment<MatcherType, PointAdapter> (options, logger, set1_point1, set2_point1, mat, sampler, visitor);
        }
        else if(point_type == 2) // extlib2::PointType2, convert read sets to vector of PointType2 to emulate PointAdapter usage
        {
          logger.Log<Utils::Verbose>("Registration on std::list of extlib2::PointType2 instances using extlib2::PointAdapter");
          using PointType    = extlib2::PointType2;
          using PointAdapter = extlib2::PointAdapter;
          using MatcherType = gr::Match4pcsBase<gr::FunctorSuper4PCS, PointAdapter, TrVisitorType, gr::AdaptivePointFilter, gr::AdaptivePointFilter::Options>;
          using OptionType  = typename MatcherType::OptionsType;

          OptionType options;
          if(! Demo::setOptionsFromArgs(options, logger))
            exit(-2); /// \FIXME use status codes for error reporting

          auto set1_point2 = getExtlib2Points(set1);
          auto set2_point2 = getExtlib2Points(set2);

          score = computeAlignment<MatcherType, PointAdapter> (options, logger, set1_point2, set2_point2, mat, sampler, visitor);

          // deallocate memory for points2
          if(!set1_point2.empty()) {
            delete[] set1_point2.front().posBuffer;
            delete[] set1_point2.front().nBuffer;
            delete[] set1_point2.front().colorBuffer;
            set1_point2.clear();
          }

          if(!set2_point2.empty()) {
            delete[] set2_point2.front().posBuffer;
            delete[] set2_point2.front().nBuffer;
            delete[] set2_point2.front().colorBuffer;
            set2_point2.clear();
          }
        }
        else
        {
          std::cerr << "Invalid point type" << std::endl;
          exit(-1);
        }
      }
      else {
        // 4PCS
        if(point_type == 0) // gr::Point3D, directly pass the read sets
        {
          using PointType    = gr::Point3D<Scalar>;
          using PointAdapter = gr::Point3D<Scalar>;
          using MatcherType  = gr::Match4pcsBase<gr::Functor4PCS, PointAdapter, TrVisitorType, gr::AdaptivePointFilter, gr::AdaptivePointFilter::Options>;
          using OptionType   = typename MatcherType::OptionsType;

          OptionType options;
          if(! Demo::setOptionsFromArgs(options, logger))
            exit(-2); /// \FIXME use status codes for error reporting

          score = computeAlignment<MatcherType, PointAdapter> (options, logger, set1, set2, mat, sampler, visitor);
        }
        else if(point_type == 1) // extlib1::PointType1, convert read sets to vector of PointType1 to emulate PointAdapter usage
        {
          logger.Log<Utils::Verbose>("Registration on std::vector of extlib1::PointType1 instances using extlib1::PointAdapter");
          using PointType    = extlib1::PointType1;
          using PointAdapter = extlib1::PointAdapter;
          using MatcherType = gr::Match4pcsBase<gr::Functor4PCS, PointAdapter, TrVisitorType, gr::AdaptivePointFilter, gr::AdaptivePointFilter::Options>;
          using OptionType  = typename MatcherType::OptionsType;

          OptionType options;
          if(! Demo::setOptionsFromArgs(options, logger))
            exit(-2); /// \FIXME use status codes for error reporting

          auto set1_point1 = getExtlib1Points(set1);
          auto set2_point1 = getExtlib1Points(set2);

          score = computeAlignment<MatcherType, PointAdapter> (options, logger, set1_point1, set2_point1, mat, sampler, visitor);
        }
        else if(point_type == 2) // extlib2::PointType2, convert read sets to vector of PointType2 to emulate PointAdapter usage
        {
          logger.Log<Utils::Verbose>("Registration on std::vector of extlib2::PointType2 instances using extlib2::PointAdapter");
          using PointType    = extlib2::PointType2;
          using PointAdapter = extlib2::PointAdapter;
          using MatcherType = gr::Match4pcsBase<gr::Functor4PCS, PointAdapter, TrVisitorType, gr::AdaptivePointFilter, gr::AdaptivePointFilter::Options>;
          using OptionType  = typename MatcherType::OptionsType;

          OptionType options;
          if(! Demo::setOptionsFromArgs(options, logger))
            exit(-2); /// \FIXME use status codes for error reporting

          auto set1_point2 = getExtlib2Points(set1);
          auto set2_point2 = getExtlib2Points(set2);

          score = computeAlignment<MatcherType, PointAdapter> (options, logger, set1_point2, set2_point2, mat, sampler, visitor);
          
          // deallocate memory for points2
          if(!set1_point2.empty()) {
            delete[] set1_point2.front().posBuffer;
            delete[] set1_point2.front().nBuffer;
            delete[] set1_point2.front().colorBuffer;
            set1_point2.clear();
          }

          if(!set2_point2.empty()) {
            delete[] set2_point2.front().posBuffer;
            delete[] set2_point2.front().nBuffer;
            delete[] set2_point2.front().colorBuffer;
            set2_point2.clear();
          }

        }
      }

  }
  catch (const std::exception& e) {
      logger.Log<Utils::ErrorReport>( "[Error]: " , e.what() );
      logger.Log<Utils::ErrorReport>( "Aborting with code -3 ..." );
      return -3;
  }
  catch (...) {
      logger.Log<Utils::ErrorReport>( "[Unknown Error]: Aborting with code -4 ..." );
      return -4;
  }


  if(! outputMat.empty() ){
      logger.Log<Utils::Verbose>( "Exporting Matrix to ",
                                  outputMat.c_str(),
                                  "..." );
      iomananger.WriteMatrix(outputMat, mat.cast<double>(), IOManager::POLYWORKS);
      logger.Log<Utils::Verbose>( "Export DONE" );
  }

  if (! output.empty() ){

      logger.Log<Utils::Verbose>( "Exporting Registered geometry to ",
                                  output.c_str(),
                                  "..." );
      Utils::TransformPointCloud(set2, mat);
      iomananger.WriteObject((char *)output.c_str(),
                             set2,
                             tex_coords2,
                             normals2,
                             tris2,
                             mtls2);
      logger.Log<Utils::Verbose>( "Export DONE" );
  }

  return 0;
}
