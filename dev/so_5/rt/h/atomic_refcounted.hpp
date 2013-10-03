/*
	SObjectizer 5.
*/

/*!
	\file
	\brief The base class for the object with a reference counting definition.
*/

#if !defined( _SO_5__RT__ATOMIC_REFCOUNTED_HPP_ )
#define _SO_5__RT__ATOMIC_REFCOUNTED_HPP_

#include <so_5/h/declspec.hpp>
#include <so_5/h/types.hpp>

#include <type_traits>

namespace so_5
{

namespace rt
{

//! The base class for the object with a reference counting.
/*!
 * Should be used as a base class. The smart reference for such objects
 * should be defined for derived classes (for example so_5::rt::agent_ref_t).
*/
class SO_5_TYPE atomic_refcounted_t
{
		/*! Disabled. */
		atomic_refcounted_t(
			const atomic_refcounted_t & );

		/*! Disabled. */
		atomic_refcounted_t &
		operator = (
			const atomic_refcounted_t & );

	public:
		//! Default constructor.
		/*!
		 * Sets reference counter to 0.
		 */
		atomic_refcounted_t();

		//! Destructor.
		/*!
		 * Do nothing.
		 */
		~atomic_refcounted_t();

		//! Increments reference count.
		inline void
		inc_ref_count()
		{
			++m_ref_counter;
		}

		//! Decrement reference count.
		/*!
		 * \return Value of reference counter *after* decrement.
		*/
		inline unsigned long
		dec_ref_count()
		{
			return --m_ref_counter;
		}

	private:
		//! Object reference count.
		atomic_counter_t m_ref_counter;
};

//
// smart_atomic_reference_t
//
/*!
 * \since v.5.2.0
 * \brief Template class for smart reference wrapper on the atomic_refcounted_t.
 *
 * \tparam T class which must be derived from the atomic_refcounted_t.
 */
template< class T >
class smart_atomic_reference_t
{
		static void ensure_right_T()
		{
			static_assert(
					std::is_base_of< atomic_refcounted_t, T >::value,
					"T must be derived from atomic_refcounted_t" );
		}

	public :
		//! Default constructor.
		/*!
		 * Constructs a null reference.
		 */
		smart_atomic_reference_t()
			:	m_obj( nullptr )
		{
			ensure_right_T();
		}
		//! Constructor for a raw pointer.
		smart_atomic_reference_t( T * obj )
			:	m_obj( obj )
		{
			ensure_right_T();
			take_object();
		}
		//! Copy constructor.
		smart_atomic_reference_t( const smart_atomic_reference_t & o )
			:	m_obj( o.m_obj )
		{
			ensure_right_T();
			take_object();
		}
		//! Move constructor.
		smart_atomic_reference_t( smart_atomic_reference_t && o )
			:	m_obj( o.m_obj )
		{
			ensure_right_T();
			o.m_obj = nullptr;
		}

		//! Destructor.
		~smart_atomic_reference_t()
		{
			dismiss_object();
		}

		//! Copy operator.
		smart_atomic_reference_t &
		operator=( const smart_atomic_reference_t & o )
		{
			smart_atomic_reference_t t( o );
			swap( t );
			return *this;
		}

		//! Move operator.
		smart_atomic_reference_t &
		operator=( smart_atomic_reference_t && o )
		{
			if( &o != this )
			{
				dismiss_object();
				m_obj = o.m_obj;
				o.m_obj = nullptr;
			}
			return *this;
		}

		//! Swap values.
		void
		swap( smart_atomic_reference_t & o )
		{
			T * t = m_obj;
			m_obj = o.m_obj;
			o.m_obj = t;
		}

		//! Is this a null reference?
		bool
		empty() const
		{
			return nullptr != m_obj;
		}

		/*!
		 * \name Access to object.
		 * \{
		 */
		T *
		get() const
		{
			return m_obj;
		}

		T *
		operator->() const
		{
			return m_obj;
		}

		T &
		operator*() const
		{
			return *m_obj;
		}
		/*!
		 * \}
		 */

		/*!
		 * \name Comparision
		 * \{
		 */
		bool operator==( const smart_atomic_reference_t & o ) const
		{
			T * p1 = get();
			T * p2 = o.get();
			if( p1 == nullptr && p2 == nullptr )
				return true;
			if( p1 != nullptr && p2 != nullptr )
				return (*p1) == (*p2);
			return false;
		}

		bool operator<( const smart_atomic_reference_t & o ) const
		{
			T * p1 = get();
			T * p2 = o.get();
			if( p1 == nullptr && p2 == nullptr )
				return false;
			if( p1 != nullptr && p2 != nullptr )
				return (*p1) < (*p2);
			if( p1 == nullptr && p2 != nullptr )
				return true;
			return false;
		}
		/*!
		 * \}
		 */

	private :
		//! Object controlled by a smart reference.
		T * m_obj;

		//! Increment reference count to object if it's not null.
		void
		take_object()
		{
			if( m_obj )
				m_obj->inc_ref_count();
		}

		//! Decrement reference count to object and delete it if needed.
		void
		dismiss_object()
		{
			if( m_obj )
				if( 0 == m_obj->dec_ref_count() )
				{
					delete m_obj;
					m_obj = nullptr;
				}
		}
};

} /* namespace rt */

} /* namespace so_5 */

#endif

