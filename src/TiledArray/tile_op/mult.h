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
 *  mult.h
 *  May 8, 2013
 *
 */

#ifndef TILEDARRAY_TILE_OP_MULT_H__INCLUDED
#define TILEDARRAY_TILE_OP_MULT_H__INCLUDED

#include <TiledArray/tile_op/permute.h>
#include <TiledArray/tensor.h>

namespace TiledArray {
  namespace math {

    /// Tile multiplication operation

    /// This multiplication operation will multiply the content two tiles and
    /// apply a permutation to the result tensor. If no permutation is given or
    /// the permutation is null, then the result is not permuted.
    /// \tparam Result The result type
    /// \tparam Left The left-hand argument type
    /// \tparam Right The right-hand argument type
    /// \tparam LeftConsumable A flag that is \c true when the left-hand
    /// argument is consumable.
    /// \tparam RightConsumable A flag that is \c true when the right-hand
    /// argument is consumable.
    /// \tparam Enabler Used to disambiguate specialization
    template <typename Result, typename Left, typename Right, bool LeftConsumable,
        bool RightConsumable, typename Enabler = void>
    class Mult {
    public:
      typedef Mult<Result, Left, Right, false, false> Mult_; ///< This object type
      typedef const Left& first_argument_type; ///< The left-hand argument type
      typedef const Right& second_argument_type; ///< The right-hand argument type
      typedef Result result_type; ///< The result tile type

    private:
      Permutation perm_; ///< The result permutation

    public:
      /// Default constructor

      /// Construct a multiplication operation that does not permute the result tile
      Mult() : perm_() { }

      /// Permute constructor

      /// Construct a multiplication operation that permutes the result tensor
      /// \param perm The permutation to apply to the result tile
      Mult(const Permutation& perm) : perm_(perm) { }

      /// Copy constructor

      /// \param other The multiplication operation object to be copied
      Mult(const Mult_& other) : perm_(other.perm_) { }

      /// Copy assignment

      /// \param other The multiplication operation object to be copied
      /// \return A reference to this object
      Mult_& operator=(const Mult_& other) {
        perm_ = other.perm_;
        return *this;
      }

      /// Multiply two non-zero tiles and possibly permute

      /// \param first The left-hand argument
      /// \param second The right-hand argument
      /// \return The sum and permutation of \c first and \c second
      result_type operator()(first_argument_type first, second_argument_type second) const {
        TA_ASSERT(first.range() == second.range());

        result_type result;
        if(perm_.dim() > 1) {
          TiledArray::detail::Multiplies<typename Left::value_type,
              typename Right::value_type, typename Result::value_type> op;

          permute(result, perm_, first, second, op);
        } else
          result = first * second;

        return result;
      }
    }; // class Mult

    /// Tile multiplication operation

    /// This multiplication operation will multiply the content two tiles and
    /// apply a permutation to the result tensor. If no permutation is given or
    /// the permutation is null, then the result is not permuted.
    /// \tparam Result The result type
    /// \tparam Right The right-hand argument type
    /// \note This specialization assumes the left-hand tile is consumable
    template <typename Result, typename Right, bool RightConsumable>
    class Mult<Result, Result, Right, true, RightConsumable, void> {
    public:
      typedef Mult<Result, Result, Right, true, false> Mult_; ///< This object type
      typedef Result first_argument_type; ///< The left-hand argument type
      typedef const Right& second_argument_type; ///< The right-hand argument type
      typedef Result result_type; ///< The result tile type

    private:
      Permutation perm_; ///< The result permutation

    public:
      /// Default constructor

      /// Construct a multiplication operation that does not permute the result tile
      Mult() : perm_() { }

      /// Permute constructor

      /// Construct a multiplication operation that permutes the result tensor
      /// \param perm The permutation to apply to the result tile
      Mult(const Permutation& perm) : perm_(perm) { }

      /// Copy constructor

      /// \param other The multiplication operation object to be copied
      Mult(const Mult_& other) : perm_(other.perm_) { }

      /// Copy assignment

      /// \param other The multiplication operation object to be copied
      /// \return A reference to this object
      Mult_& operator=(const Mult_& other) {
        perm_ = other.perm_;
        return *this;
      }

      /// Multiply two non-zero tiles and possibly permute

      /// \param first The left-hand argument
      /// \param second The right-hand argument
      /// \return The sum and permutation of \c first and \c second
      result_type operator()(first_argument_type first, second_argument_type second) const {
        TA_ASSERT(first.range() == second.range());

        if(perm_.dim() > 1) {
          TiledArray::detail::Multiplies<typename Result::value_type,
              typename Right::value_type, typename Result::value_type> op;

          result_type result;
          permute(result, perm_, first, second, op);
          return result;
        } else {
          first *= second;
          return first;
        }
      }
    }; // class Mult


    /// Tile multiplication operation

    /// This multiplication operation will multiply the content two tiles and
    /// apply a permutation to the result tensor. If no permutation is given or
    /// the permutation is null, then the result is not permuted.
    /// \tparam Result The result type
    /// \tparam Left The left-hand argument type
    /// \note This specialization assumes the right-hand tile is consumable
    template <typename Result, typename Left, bool LeftConsumable>
    class Mult<Result, Left, Result, LeftConsumable, true,
        typename madness::disable_if_c<LeftConsumable && std::is_same<Result, Left>::value>::type>
   {
    public:
      typedef Mult<Result, Left, Result, true, false> Mult_; ///< This object type
      typedef const Left& first_argument_type; ///< The left-hand argument type
      typedef Result second_argument_type; ///< The right-hand argument type
      typedef Result result_type; ///< The result tile type

    private:
      Permutation perm_; ///< The result permutation

    public:
      /// Default constructor

      /// Construct a multiplication operation that does not permute the result tile
      Mult() : perm_() { }

      /// Permute constructor

      /// Construct a multiplication operation that permutes the result tensor
      /// \param perm The permutation to apply to the result tile
      Mult(const Permutation& perm) : perm_(perm) { }

      /// Copy constructor

      /// \param other The multiplication operation object to be copied
      Mult(const Mult_& other) : perm_(other.perm_) { }

      /// Copy assignment

      /// \param other The multiplication operation object to be copied
      /// \return A reference to this object
      Mult_& operator=(const Mult_& other) {
        perm_ = other.perm_;
        return *this;
      }

      /// Multiply two non-zero tiles and possibly permute

      /// \param first The left-hand argument
      /// \param second The right-hand argument
      /// \return The sum and permutation of \c first and \c second
      result_type operator()(first_argument_type first, second_argument_type second) const {
        TA_ASSERT(first.range() == second.range());


        if(perm_.dim() > 1) {
          TiledArray::detail::Multiplies<typename Left::value_type,
              typename Result::value_type, typename Result::value_type> op;
          result_type result;
          permute(result, perm_, first, second, op);
          return result;
        } else {
          second *= first;
          return second;
        }
      }
    }; // class Mult

  } // namespace math
} // namespace TiledArray

#endif // TILEDARRAY_TILE_OP_MULT_H__INCLUDED
