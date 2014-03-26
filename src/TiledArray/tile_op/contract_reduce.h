/*
 *  This file is a part of TiledArray.
 *  Copyright (C) 2013  Virginia Tech
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Justus Calvin
 *  Department of Chemistry, Virginia Tech
 *
 *  contract_reduce.h
 *  Oct 9, 2013
 *
 */

#ifndef TILEDARRAY_CONTRACT_REDUCE_H__INCLUDED
#define TILEDARRAY_CONTRACT_REDUCE_H__INCLUDED

#include <TiledArray/error.h>
#include <TiledArray/tile_op/permute.h>
#include <TiledArray/math/gemm_helper.H>

namespace TiledArray {
  namespace math {

    /// Contract and reduce operation

    /// This object uses a tile contraction operation to form a pair reduction
    /// operation.
    template <typename Result, typename Left, typename Right>
    class ContractReduce {
    public:
      typedef ContractReduce ContractReduce_; ///< This class type
      typedef const Left& first_argument_type; ///< The left tile type
      typedef const Right& second_argument_type; ///< The right tile type
      typedef Result result_type; ///< The result tile type.
      typedef typename TiledArray::detail::scalar_type<result_type>::type scalar_type;

    private:

      struct Impl {
        Impl(const madness::cblas::CBLAS_TRANSPOSE left_op,
            const madness::cblas::CBLAS_TRANSPOSE right_op, const scalar_type alpha,
            const unsigned int result_rank, const unsigned int left_rank,
            const unsigned int right_rank, const Permutation& perm = Permutation()) :
          gemm_helper_(left_op, right_op, result_rank, left_rank, right_rank),
          alpha_(alpha), perm_(perm)
        { }

        GemmHelper gemm_helper_; ///< Gemm helper object
        scalar_type alpha_; ///< Scaling factor applied to the contraction of the left- and right-hand arguments
        Permutation perm_; ///< Permutation that is applied to the final result tensor
      };

      std::shared_ptr<Impl> pimpl_;

    public:

      /// Default constructor
      ContractReduce() : pimpl_() { }

      /// Construct contract/reduce functor

      /// \param left_op The left-hand BLAS matrix operation
      /// \param right_op The right-hand BLAS matrix operation
      /// \param result_rank The rank of the result tensor
      /// \param left_rank The rank of the left-hand tensor
      /// \param right_rank The rank of the right-hand tensor
      /// \param perm The permutation to be applied to the result tensor
      /// (default = no permute)
      ContractReduce(const madness::cblas::CBLAS_TRANSPOSE left_op,
          const madness::cblas::CBLAS_TRANSPOSE right_op, const scalar_type alpha,
          const unsigned int result_rank, const unsigned int left_rank,
          const unsigned int right_rank, const Permutation& perm = Permutation()) :
        pimpl_(new Impl(left_op, right_op, alpha, result_rank, left_rank, right_rank, perm))
      { }

      /// Functor copy constructor

      /// Shallow copy of this functor
      /// \param other The functor to be copied
      ContractReduce(const ContractReduce_& other) :
        pimpl_(other.pimpl_)
      { }

      /// Functor assignment operator

      /// Shallow copy of this functor
      /// \param other The functor to be copied
      ContractReduce_& operator=(const ContractReduce_& other) {
        pimpl_ = other.pimpl_;
        return *this;
      }

      /// Gemm meta data accessor

      /// \return A reference to the gemm helper object
      const GemmHelper& gemm_helper() const {
        TA_ASSERT(pimpl_);
        return pimpl_->gemm_helper_;
      }

      /// Compute the number of contracted ranks

      /// \return The number of ranks that are summed by this operation
      unsigned int num_contract_ranks() const {
        TA_ASSERT(pimpl_);
        return pimpl_->gemm_helper_.num_contract_ranks();
      }

      /// Result rank accessor

      /// \return The rank of the result tile
      unsigned int result_rank() const {
        TA_ASSERT(pimpl_);
        return pimpl_->gemm_helper_.result_rank();
      }

      /// Left-hand argument rank accessor

      /// \return The rank of the left-hand tile
      unsigned int left_rank() const {
        TA_ASSERT(pimpl_);
        return pimpl_->gemm_helper_.left_rank();
      }

      /// Right-hand argument rank accessor

      /// \return The rank of the right-hand tile
      unsigned int right_rank() const {
        TA_ASSERT(pimpl_);
        return pimpl_->gemm_helper_.right_rank();
      }

      /// Create a result type object

      /// Initialize a result object for subsequent reductions
      result_type operator()() const {
        return result_type();
      }

      /// Post processing step
      result_type operator()(const result_type& temp) const {
        TA_ASSERT(pimpl_);

        result_type result;

        if(! temp.empty()) {
          if(pimpl_->perm_.dim() < 1u)
            result = temp;
          else
            TiledArray::math::permute(result, pimpl_->perm_, temp);
        }

        return result;
      }

      /// Reduce two result objects

      /// Add \c arg to \c result .
      /// \param[in,out] result The result object that will be the reduction target
      /// \param[in] arg The argument that will be added to \c result
      void operator()(result_type& result, const result_type& arg) const {
        result.add_to(arg);
      }

      /// Contract a pair of tiles and add to a target tile

      /// Contract \c left and \c right and add the result to \c result.
      /// \param[in,out] result The result object that will be the reduction target
      /// \param[in] left The left-hand tile to be contracted
      /// \param[in] right The right-hand tile to be contracted
      void operator()(result_type& result, first_argument_type left,
          second_argument_type right) const
      {
        TA_ASSERT(pimpl_);

        if(result.empty())
          result = left.gemm(right, pimpl_->alpha_, pimpl_->gemm_helper_);
        else
          result.gemm(left, right, pimpl_->alpha_, pimpl_->gemm_helper_);
      }

      /// Contract a pair of tiles and add to a target tile

      /// Contract \c left1 with \c right1 and \c left2 with \c right2 ,
      /// and add the two results.
      /// \param[in,out] result The object that will hold the result of this
      /// reduction operation.
      /// \param[in] left1 The first left-hand tile to be contracted
      /// \param[in] right1 The first right-hand tile to be contracted
      /// \param[in] left2 The second left-hand tile to be contracted
      /// \param[in] right2 The second right-hand tile to be contracted
      void operator()(result_type& result,
          first_argument_type left1, second_argument_type right1,
          first_argument_type left2, second_argument_type right2) const
      {
        TA_ASSERT(pimpl_);

        if(result.empty())
          result = left1.gemm(right1, pimpl_->alpha_, pimpl_->gemm_helper_);
        else
          result.gemm(left1, right1, pimpl_->alpha_, pimpl_->gemm_helper_);

        result.gemm(left2, right2, pimpl_->alpha_, pimpl_->gemm_helper_);
      }

    }; // class ContractReduce

  }  // namespace math
} // namespace TiledArray

#endif // TILEDARRAY_CONTRACT_REDUCE_H__INCLUDED
