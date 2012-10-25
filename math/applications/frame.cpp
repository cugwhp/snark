#include <boost/thread/thread_time.hpp>
#include <snark/math/rotation_matrix.h>
#include <snark/math/applications/frame.h>
#include "frame.h"

namespace snark{ namespace applications {

frame::frame( const position& p, bool to, bool interpolate, bool rotation_present )
    : outputframe( false )
    , m_to( to )
    , m_interpolate( interpolate )
    , m_rotation( ::Eigen::Matrix3d::Identity() )
    , m_discarded( false )
    , rotation_present_( rotation_present )
{
    set_position( p );
}

frame::frame( const comma::csv::options& options, bool discardOutOfOrder, boost::optional< boost::posix_time::time_duration > maxGap, bool outputframe, bool to, bool interpolate, bool rotation_present )
    : outputframe( outputframe )
    , m_to( to )
    , m_interpolate( interpolate )
    , m_rotation( ::Eigen::Matrix3d::Identity() )
    , m_istream( new comma::io::istream( options.filename, options.binary() ? comma::io::mode::binary : comma::io::mode::ascii, comma::io::mode::blocking ) )
    , m_discardOutOfOrder( discardOutOfOrder )
    , m_maxGap( maxGap )
    , rotation_present_( rotation_present )
{
    m_is.reset( new comma::csv::input_stream< position_type >( *( *m_istream )(), options ) );
    const position_type* p = m_is->read(); // todo: maybe not very good, move to value()
    if( p == NULL ) { COMMA_THROW( comma::exception, "failed to read from " << options.filename ); }
    m_pair.first = *p;
    set_position( p->value );
    p = m_is->read(); // todo: maybe not very good, move to value()
    if( p == NULL ) { COMMA_THROW( comma::exception, "failed to read from " << options.filename ); }
    m_pair.second = *p;
}

frame::~frame()
{
    if( m_istream ) { m_istream->close(); }
}

const frame::point_type* frame::converted( const point_type& rhs )
{
    if( !m_is ) { return &convert( rhs ); }
    m_discarded = false;
    while( rhs.t >= m_pair.first.t )
    {
        if( ( rhs.t <= m_pair.second.t ) )
        {
            if( m_interpolate )
            {
                if( m_pair.first.t == m_pair.second.t )
                {
                    set_position( m_pair.second.value );
                }
                else
                {
                    double factor = double( ( rhs.t - m_pair.first.t ).total_microseconds() ) / ( m_pair.second.t - m_pair.first.t ).total_microseconds();
                    position p( m_pair.first.value.coordinates * ( 1 - factor ) + m_pair.second.value.coordinates * factor,
                                m_pair.first.value.orientation * ( 1 - factor ) + m_pair.second.value.orientation * factor );
                    set_position( p ); // TODO do interpolation with eigen ?
                }
            }
            else
            {
                // keep the first point or take the second point if closer
                if( ( rhs.t - m_pair.first.t ).total_microseconds() * 2u  > ( m_pair.second.t - m_pair.first.t ).total_microseconds() )
                {
                    set_position( m_pair.second.value );
                }
            }
            return &convert( rhs );
        }
        do
        {
            const position_type* p = m_is->read();
            if( p == NULL ) { return NULL; }
            m_pair.first = m_pair.second;
            m_pair.second = *p;
            if( !m_interpolate ) { set_position( m_pair.first.value ); }
        }
        while( m_maxGap && ( m_pair.second.t - m_pair.first.t ) > *m_maxGap );
    }
    if( !m_discardOutOfOrder ) { COMMA_THROW( comma::exception, "expected timestamp not earlier than " << boost::posix_time::to_iso_string( m_pair.first.t ) << "; got " << boost::posix_time::to_iso_string( rhs.t ) << "; use --discard" ); }
    m_discarded = true;
    return NULL;
}

void frame::set_position( const position& p )
{
    m_position = p;
    m_translation.vector() = ::Eigen::Vector3d( p.coordinates );
    m_rotation = snark::rotation_matrix::rotation( p.orientation );
    m_transform = m_to
                ? m_rotation.transpose() * m_translation.inverse()
                : m_translation * m_rotation;
}


const frame::point_type& frame::convert( const point_type& rhs )
{
    m_converted.t = rhs.t;
    m_converted.value.coordinates = m_transform * rhs.value.coordinates;
    if( rotation_present_ )
    {
        Eigen::Matrix3d m = snark::rotation_matrix::rotation( rhs.value.orientation );
        if( m_to ) { m = m_rotation.transpose() * m; }
        else { m = m_rotation * m; }
        m_converted.value.orientation = snark::rotation_matrix::roll_pitch_yaw( m );
    }
    return m_converted;
}

} } // namespace snark{ namespace applications {