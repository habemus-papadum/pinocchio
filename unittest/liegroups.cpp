// Copyright (c) 2017, Joseph Mirabel
// Authors: Joseph Mirabel (joseph.mirabel@laas.fr)
//
// This file is part of pinocchio.
// pinocchio is free software: you can redistribute it
// and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// pinocchio is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Lesser Public License for more details.  You should have
// received a copy of the GNU Lesser General Public License along with
// pinocchio. If not, see <http://www.gnu.org/licenses/>.

#include "pinocchio/multibody/liegroup/liegroup.hpp"

#include "pinocchio/multibody/joint/joint.hpp"

#include <boost/test/unit_test.hpp>
#include <boost/utility/binary.hpp>

using namespace se3;

template <typename T>
void test_lie_group_methods (T & jmodel, typename T::JointDataDerived &)
{
    std::cout << "Testing Joint over " << jmodel.shortname() << std::endl;
    typedef typename T::ConfigVector_t  ConfigVector_t;
    typedef typename T::TangentVector_t TangentVector_t;

    ConfigVector_t  q1(ConfigVector_t::Random (jmodel.nq()));
    TangentVector_t q1_dot(TangentVector_t::Random (jmodel.nv()));
    ConfigVector_t  q2(ConfigVector_t::Random (jmodel.nq()));
    double u = 0.3;
    // se3::Inertia::Matrix6 Ia(se3::Inertia::Random().matrix());
    // bool update_I = false;

    q1 = jmodel.random();
    q2 = jmodel.random();

    // jmodel.calc(jdata, q1, q1_dot);
    // jmodel.calc_aba(jdata, Ia, update_I);

    typedef typename LieGroup<T>::type LieGroupType;
    // LieGroupType lg;

    static Eigen::VectorXd Ones (Eigen::VectorXd::Ones(jmodel.nq()));

    std::string error_prefix("LieGroup ");
    BOOST_CHECK_MESSAGE(jmodel.nq() == LieGroupType::NQ, std::string(error_prefix + " - nq "));
    BOOST_CHECK_MESSAGE(jmodel.nv() == LieGroupType::NV, std::string(error_prefix + " - nv "));

    BOOST_CHECK_MESSAGE(jmodel.integrate(q1,q1_dot).isApprox(LieGroupType::integrate(q1,q1_dot)) ,std::string(error_prefix + " - integrate "));
    BOOST_CHECK_MESSAGE(jmodel.interpolate(q1,q2,u).isApprox(LieGroupType::interpolate(q1,q2,u)) ,std::string(error_prefix + " - interpolate "));
    BOOST_CHECK_MESSAGE
      (jmodel.randomConfiguration( -1 * Ones, Ones).size() ==
       LieGroupType().randomConfiguration(-1 * Ones, Ones).size(),
       std::string(error_prefix + " - RandomConfiguration dimensions "));
    BOOST_CHECK_MESSAGE(jmodel.difference(q1,q2).isApprox(LieGroupType::difference(q1,q2)) ,std::string(error_prefix + " - difference "));
    BOOST_CHECK_MESSAGE(fabs(jmodel.distance(q1,q2) - LieGroupType::distance(q1,q2)) < 1e-12 ,std::string(error_prefix + " - distance "));
}

struct TestJoint{

  template <typename T>
  void operator()(const T ) const
  {
    T jmodel;
    jmodel.setIndexes(0,0,0);
    typename T::JointDataDerived jdata = jmodel.createData();

    test_lie_group_methods(jmodel, jdata);    
  }

  void operator()(const se3::JointModelRevoluteUnaligned & ) const
  {
    se3::JointModelRevoluteUnaligned jmodel(1.5, 1., 0.);
    jmodel.setIndexes(0,0,0);
    se3::JointModelRevoluteUnaligned::JointDataDerived jdata = jmodel.createData();

    test_lie_group_methods(jmodel, jdata);
  }

  void operator()(const se3::JointModelPrismaticUnaligned & ) const
  {
    se3::JointModelPrismaticUnaligned jmodel(1.5, 1., 0.);
    jmodel.setIndexes(0,0,0);
    se3::JointModelPrismaticUnaligned::JointDataDerived jdata = jmodel.createData();

    test_lie_group_methods(jmodel, jdata);
  }

};

BOOST_AUTO_TEST_SUITE ( BOOST_TEST_MODULE )

BOOST_AUTO_TEST_CASE ( test_all_liegroups )
{
  typedef boost::variant< JointModelRX, JointModelRY, JointModelRZ, JointModelRevoluteUnaligned
                          , JointModelSpherical, JointModelSphericalZYX
                          , JointModelPX, JointModelPY, JointModelPZ
                          , JointModelPrismaticUnaligned
                          , JointModelFreeFlyer
                          , JointModelPlanar
                          , JointModelTranslation
                          , JointModelRUBX, JointModelRUBY, JointModelRUBZ
                          > Variant;
  boost::mpl::for_each<Variant::types>(TestJoint());
  // FIXME JointModelComposite does not work.
  // boost::mpl::for_each<JointModelVariant::types>(TestJoint());
}

BOOST_AUTO_TEST_CASE ( test_vector_space )
{
  typedef VectorSpaceOperation<3> VSO_t;
  VSO_t::ConfigVector_t q,
    lo(VSO_t::ConfigVector_t::Constant(-std::numeric_limits<double>::infinity())),
    // lo(VSO_t::ConfigVector_t::Constant(                                       0)),
    // up(VSO_t::ConfigVector_t::Constant( std::numeric_limits<double>::infinity()));
    up(VSO_t::ConfigVector_t::Constant(                                       0));

  bool error = false;
  try {
    VSO_t ().randomConfiguration(lo, up, q);
  } catch (const std::runtime_error&) {
    error = true;
  }
  BOOST_CHECK_MESSAGE(error, "Random configuration between infinite bounds should return an error");
}

BOOST_AUTO_TEST_CASE ( test_size )
{
  // R^1: neutral = [0]
  VectorSpaceOperation <1> vs1;
  Eigen::VectorXd neutral;
  neutral.resize (1);
  neutral.setZero ();
  BOOST_CHECK (vs1.nq () == 1);
  BOOST_CHECK (vs1.nv () == 1);
  BOOST_CHECK (vs1.name () == "R^1");
  BOOST_CHECK (vs1.neutral () == neutral);
  // R^2: neutral = [0, 0]
  VectorSpaceOperation <2> vs2;
  neutral.resize (2);
  neutral.setZero ();
  BOOST_CHECK (vs2.nq () == 2);
  BOOST_CHECK (vs2.nv () == 2);
  BOOST_CHECK (vs2.name () == "R^2");
  BOOST_CHECK (vs2.neutral () == neutral);
  // R^3: neutral = [0, 0, 0]
  VectorSpaceOperation <3> vs3;
  neutral.resize (3);
  neutral.setZero ();
  BOOST_CHECK (vs3.nq () == 3);
  BOOST_CHECK (vs3.nv () == 3);
  BOOST_CHECK (vs3.name () == "R^3");
  BOOST_CHECK (vs3.neutral () == neutral);
  // SO(2): neutral = [1, 0]
  SpecialOrthogonalOperation <2> so2;
  neutral.resize (2); neutral [0] = 1; neutral [1] = 0;
  BOOST_CHECK (so2.nq () == 2);
  BOOST_CHECK (so2.nv () == 1);
  BOOST_CHECK (so2.name () == "SO(2)");
  BOOST_CHECK (so2.neutral () == neutral);
  // SO(3): neutral = [0, 0, 0, 1]
  SpecialOrthogonalOperation <3> so3;
  neutral.resize (4); neutral.setZero ();
  neutral [3] = 1;
  BOOST_CHECK (so3.nq () == 4);
  BOOST_CHECK (so3.nv () == 3);
  BOOST_CHECK (so3.name () == "SO(3)");
  BOOST_CHECK (so3.neutral () == neutral);
  // SE(2): neutral = [0, 0, 1, 0]
  SpecialEuclideanOperation <2> se2;
  neutral.resize (4); neutral.setZero ();
  neutral [2] = 1;
  BOOST_CHECK (se2.nq () == 4);
  BOOST_CHECK (se2.nv () == 3);
  BOOST_CHECK (se2.name () == "SE(2)");
  BOOST_CHECK (se2.neutral () == neutral);
  // SE(3): neutral = [0, 0, 0, 0, 0, 0, 1]
  SpecialEuclideanOperation <3> se3;
  neutral.resize (7); neutral.setZero ();
  neutral [6] = 1;
  BOOST_CHECK (se3.nq () == 7);
  BOOST_CHECK (se3.nv () == 6);
  BOOST_CHECK (se3.name () == "SE(3)");
  BOOST_CHECK (se3.neutral () == neutral);
  // R^2 x SO(2): neutral = [0, 0, 1, 0]
  CartesianProductOperation <VectorSpaceOperation <2>,
                             SpecialOrthogonalOperation <2> > r2xso2;
  neutral.resize (4); neutral.setZero ();
  neutral [2] = 1;
  BOOST_CHECK (r2xso2.nq () == 4);
  BOOST_CHECK (r2xso2.nv () == 3);
  BOOST_CHECK (r2xso2.name () == "R^2*SO(2)");
  BOOST_CHECK (r2xso2.neutral () == neutral);
  // R^3 x SO(3): neutral = [0, 0, 0, 0, 0, 0, 1]
  CartesianProductOperation <VectorSpaceOperation <3>,
                             SpecialOrthogonalOperation <3> > r3xso3;
  neutral.resize (7); neutral.setZero ();
  neutral [6] = 1;
  BOOST_CHECK (r3xso3.nq () == 7);
  BOOST_CHECK (r3xso3.nv () == 6);
  BOOST_CHECK (r3xso3.name () == "R^3*SO(3)");
  BOOST_CHECK (r3xso3.neutral () == neutral);
}
BOOST_AUTO_TEST_SUITE_END ()
