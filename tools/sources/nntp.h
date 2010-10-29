/** 
 @cond
 #########################################################################
 # GPL License                                                           #
 #                                                                       #
 # This file is part of the Machine Learning Framework.                  #
 # Copyright (c) 2010, Philipp Kraus, <philipp.kraus@flashpixx.de>       #
 # This program is free software: you can redistribute it and/or modify  #
 # it under the terms of the GNU General Public License as published by  #
 # the Free Software Foundation, either version 3 of the License, or     #
 # (at your option) any later version.                                   #
 #                                                                       #
 # This program is distributed in the hope that it will be useful,       #
 # but WITHOUT ANY WARRANTY; without even the implied warranty of        #
 # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
 # GNU General Public License for more details.                          #
 #                                                                       #
 # You should have received a copy of the GNU General Public License     #
 # along with this program.  If not, see <http://www.gnu.org/licenses/>. #
 #########################################################################
 @endcond
 **/

#ifdef SOURCES

#ifndef MACHINELEARNING_TOOLS_SOURCES_NNTP_H
#define MACHINELEARNING_TOOLS_SOURCES_NNTP_H

#include <string>
#include <iostream>
#include <istream>
#include <ostream>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/algorithm/string.hpp> 
#include <boost/iostreams/concepts.hpp>
#include <boost/iostreams/operations.hpp> 
#include <boost/iostreams/filtering_stream.hpp>


#include "../../exception/exception.h"
#include "../language/language.h"


namespace machinelearning { namespace tools { namespace sources {
    
    namespace bip  = boost::asio::ip;
    namespace bio  = boost::iostreams;

    
    /** class for creating nntp connection (exspecially for creating distance matrix).
     * The class is a nntp client class without any post functionality (implements the RFC 3977)
     * @see http://tools.ietf.org/html/rfc3977 [old RFC http://www.w3.org/Protocols/rfc977/rfc977]
     * @todo ssl connection
     **/
    class nntp {
        
        public :
        
            enum content
            {
                full    = 0,
                body    = 1,
                header  = 2
            };
                
            nntp( const std::string&, const std::string& = "nntp" );
            std::string getServer( void ) const;
            std::string getPortProtocoll( void ) const;
            std::map<std::string, std::size_t> getGroupList( void );
            std::vector<std::string> getArticleIDs( const std::string& );
            void setGroup( const std::string& );
            std::string getArticle( const content& = body );
            std::string getArticle( const std::string&, const content& = body );
            std::string getArticle( const std::string&, const std::string&, const content& = body );
            std::vector<std::string> getArticle( const std::string&, const std::vector<std::string>&, const content& = body );
            std::vector<std::string> getArticle( const std::vector<std::string>&, const content& = body );
            bool existArticle( const std::string& );
            bool existArticle( const std::string&, const std::string& );
            bool nextArticle( void );
            ~nntp( void );
        
        
        private :
        
            /** string with server name **/
            const std::string m_server;
            /** port or protocoll - should be "nntp" **/
            const std::string m_portprotocoll;
            /** io service objekt for resolving the server name**/
            boost::asio::io_service m_io;
            /** socket objekt for send / receive the data **/
            bip::tcp::socket m_socket;    
        
            unsigned int send( const std::string&, const bool& = true );
            void throwNNTPError( const unsigned int& ) const;
            std::string getResponseData( void );
        
    };


    
    
    /** constructor
     * @param p_server servername or adress
     * @param p_portprotocoll port or protocoll name - shoudl be "nntp"
     **/
    inline nntp::nntp( const std::string& p_server, const std::string& p_portprotocoll ) :
        m_server( p_server ),
        m_portprotocoll( p_portprotocoll ),
        m_io(),
        m_socket(m_io)
    {
        // create resolver for server
        bip::tcp::resolver l_resolver(m_io);
        bip::tcp::resolver::query l_query(m_server, m_portprotocoll);
               
        // try to connect the server
        bip::tcp::resolver::iterator l_endpoint = l_resolver.resolve( l_query );
        bip::tcp::resolver::iterator l_endpointend;
        boost::system::error_code l_error       = boost::asio::error::host_not_found;
        
        while (l_error && l_endpoint != l_endpointend) {
            m_socket.close();
            m_socket.connect(*l_endpoint++, l_error);
        }
        
        if (l_error)
            throw exception::parameter(_("cannot connect to news server"));
        
        // read welcome line
        boost::asio::streambuf l_response;
        std::istream l_response_stream( &l_response );
        boost::asio::read_until(m_socket, l_response, "\r\n" );
    }
    
    
    /** destructor for closing the connection **/
    inline nntp::~nntp( void )
    {
        send("quit");
        
        m_socket.close();
    }
    
    
    /** returns the server name
     * @return server name
     **/
    inline std::string nntp::getServer( void ) const
    {
        return m_server;
    }
    
    
    /** returns the protocoll / port 
     * @return protocoll port
     **/
    inline std::string nntp::getPortProtocoll( void ) const
    {
        return m_portprotocoll;
    }
    
    
    /** create an exception on the status code
     * @param p_status status code
     **/
    inline void nntp::throwNNTPError( const unsigned int& p_status ) const
    {
        switch (p_status) {
            case 0   : throw exception::parameter(_("error while reading socket data"));     
                
                // nntp errors
            case 411 : throw exception::parameter(_("no such group"));                       
            case 412 : throw exception::parameter(_("no newsgroup has been selected"));      
            case 420 : throw exception::parameter(_("no article has been selected"));          
            case 421 : throw exception::parameter(_("no next article found"));               
            case 422 : throw exception::parameter(_("no previous article found"));           
            case 423 : throw exception::parameter(_("no such article number in this group")); 
            case 430 : throw exception::parameter(_("no such article found"));                
            case 435 : throw exception::parameter(_("article not wanted - do not send"));     
            case 436 : throw exception::parameter(_("transfer failed - try again later"));    
            case 437 : throw exception::parameter(_("article rejected - do not try again"));  
            case 440 : throw exception::parameter(_("posting not allowed"));                  
            case 441 : throw exception::parameter(_("posting failed"));                       
                
                // default errors
            case 500 : throw exception::parameter(_("command not recognized"));               
            case 501 : throw exception::parameter(_("command syntax error"));                 
            case 502 : throw exception::parameter(_("access restriction or permission denied")); 
            case 503 : throw exception::parameter(_("program fault"));                           
        }
    }
    
    
    /** sends a command to the nntp server and checks the returing status code
     * @param p_cmd nntp command
     * @param p_throw bool for throwing error
     * @return status code
     **/
    inline unsigned int nntp::send( const std::string& p_cmd, const bool& p_throw )
    {
        // send the command
        boost::asio::streambuf l_request;
        std::ostream l_send( &l_request );
        
        l_send << p_cmd << "\r\n" ;
        
        boost::asio::write(m_socket, l_request);
 
        
        // read the first line
        boost::asio::streambuf l_response;
        std::istream l_response_stream( &l_response );

        boost::asio::read_until(m_socket, l_response, "\r\n" );

        // copy the return value into a string and seperates the status code
        unsigned int l_status = 0;
        std::string l_returnline;
        bio::filtering_ostream  l_out( std::back_inserter(l_returnline) );
        bio::copy( l_response, l_out );
        
        try {
            l_status = boost::lexical_cast<unsigned int>( l_returnline.substr(0,3) );
        } catch (...) {}
 
        if ( p_throw )
            throwNNTPError( l_status );
        
        return l_status;
    }

    
    /** reads the response of the socket
     * @return response via string
     **/
    inline std::string nntp::getResponseData( void )
    {
        // read data into response after the last entry is a "dot CR/LR"
        boost::asio::streambuf l_response;
        boost::asio::read_until(m_socket, l_response, "\r\n.\r\n");
        
        
        // convert stream data into string and remove the end seperator
        std::istream l_response_stream( &l_response );
        std::string l_data( (std::istreambuf_iterator<char>(l_response_stream)), std::istreambuf_iterator<char>());
        l_data.erase( l_data.end()-5, l_data.end() );
        
        return l_data;
    }
    
    
    /** fetchs the active group list
     * @return map with group names and article count
     **/
    inline std::map<std::string, std::size_t> nntp::getGroupList( void )
    {
        send("list active");
        const std::string l_data = getResponseData();
        
        // seperates the string data (remove fist and last element)
        std::vector<std::string> l_list;
        boost::split( l_list, l_data, boost::is_any_of("\n") );
        l_list.erase( l_list.begin(), l_list.begin()+1 );
        l_list.erase( l_list.end()-2, l_list.end() );
        
        // create group list with number of articles
        std::map<std::string, std::size_t> l_groups;
        for(std::size_t i=0; i < l_list.size(); ++i) {
            std::vector<std::string> l_data;
            boost::split( l_data, l_list[i], boost::is_any_of(" ") );
            
            
            if (l_data.size() > 1) {
                std::size_t l_num = 0;
                try {
                    l_num = boost::lexical_cast<unsigned int>( l_data[1] );
                } catch (...) {}
                
                l_groups[l_data[0]] = l_num;
            }
        }
        
        return l_groups;
    }
    
    
    /** return the number of articles within a group
     * @param p_group group name
     * @return vector with IDs of the articles
     **/
    inline std::vector<std::string> nntp::getArticleIDs( const std::string& p_group )
    {
        send("listgroup "+p_group);
        const std::string l_data = getResponseData();
        
        // seperates the string data (remove fist and last element)
        std::vector<std::string> l_id;
        boost::split( l_id, l_data, boost::is_any_of("\n") );
        l_id.erase( l_id.begin(), l_id.begin()+1 );
        l_id.erase( l_id.end()-2, l_id.end() );
        
        return l_id;
    }

    
    /** returns an article
     * @param p_group newsgroup
     * @param p_articleid article ID (not message id)
     * @param p_content switch for reading full article, head or body only (default body only)
     * @return message
     **/
    inline std::string nntp::getArticle( const std::string& p_group, const std::string& p_articleid, const content& p_content )
    {
        send("group "+p_group);
        
        switch (p_content) {
            case full   :   send("article "+p_articleid);   break;
            case body   :   send("body "+p_articleid);      break;
            case header :   send("head "+p_articleid);      break;
        }
        
        return getResponseData();
    }
    
    
    /** returns an article data
     * @param p_content switch for reading full article, head or body only (default body only)
     * @return message
     **/
    inline std::string nntp::getArticle( const content& p_content )
    {
        switch (p_content) {
            case full   :   send("article");   break;
            case body   :   send("body");      break;
            case header :   send("head");      break;
        }
        
        return getResponseData();
    }
    
    
    /** reads grouparticles
     * @param p_group string with group name
     * @param p_articleid std::vector with article IDs within the group (not message id)
     * @param p_content switch for reading full article, head or body only (default body only)
     * @return std::vector with string content
     **/
    std::vector<std::string> nntp::getArticle( const std::string& p_group, const std::vector<std::string>& p_articleid, const content& p_content )
    {
        send("group "+p_group);
        
        std::string l_cmd;
        switch (p_content) {
            case full   :   l_cmd = "article";   break;
            case body   :   l_cmd = "body";      break;
            case header :   l_cmd = "head";      break;
        }
        
        std::vector<std::string> l_data;
        for(std::size_t i=0; i < p_articleid.size(); ++i) {
            send(l_cmd + " " + p_articleid[i]);
            l_data.push_back( getResponseData() );
        }
        
        return l_data;
    }
    
    
    /** reads a news article
     * @param p_messageid message ID
     * @param p_content switch for reading full article, head or body only (default body only)
     * @return string with article
     **/
    inline std::string nntp::getArticle( const std::string& p_messageid, const content& p_content )
    {
        switch (p_content) {
            case full   :   send("article "+ p_messageid);   break;
            case body   :   send("body "+ p_messageid);      break;
            case header :   send("head "+ p_messageid);      break;
        }

        return getResponseData();
    }    
    
    
    /** reads an article list witin their messages IDs
     * @param p_messageid std::vector with a list of message IDs
     * @param p_content switch for reading full article, head or body only (default body only)
     * @return std::vector with messages
     **/
    inline std::vector<std::string> nntp::getArticle( const std::vector<std::string>& p_messageid, const content& p_content )
    {
        std::vector<std::string> l_data;
        
        for (std::size_t i=0; i < p_messageid.size(); ++i)
            l_data.push_back( getArticle(p_messageid[i], p_content) );

        return l_data;
    }
    
    
    /** check if article exists
     * @param p_messageid message ID
     * @return bool if exists
     **/
    bool nntp::existArticle( const std::string& p_messageid )
    {
        const unsigned int l_stat = send("stat "+p_messageid, false);
        switch (l_stat) {
            case 223 :  return true;
            case 430 :  return false;
            default  :  throwNNTPError(l_stat);
        }
        
        return false;
    }
    
    /** check if article exists within a group
     * @param p_group group
     * @param p_articleid article ID
     * @return bool if exists
     **/
    bool nntp::existArticle( const std::string& p_group, const std::string& p_articleid )
    {
        send("group "+p_group);
        
        const unsigned int l_stat = send("stat "+p_articleid, false);
        switch (l_stat) {
            case 223 :  return true;
            case 423 :  return false;
            default  :  throwNNTPError(l_stat);
        }
        
        return false;
    }
    
    
    /** sets the group 
     * @param p_group group name
     **/
    inline void nntp::setGroup( const std::string& p_group )
    {
        send("group "+p_group);
    }
    
    /** try to set the next article
     * @return bool, if it set is correct
     **/
    inline bool nntp::nextArticle( void )
    {
        const unsigned int l_stat = send("next", false);
        
        if (l_stat == 421)
            return false;
        else
            throwNNTPError(l_stat);
    
        return true;
    }
    

};};};

#endif
#endif
