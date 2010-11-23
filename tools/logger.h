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


#ifndef MACHINELEARNING_TOOLS_LOGGER_H
#define MACHINELEARNING_TOOLS_LOGGER_H

#include <string>


namespace machinelearning { namespace tools { 

    /** logger class for writing log information 
     * @todo writer method
     * @todo thread safe
     * @todo MPI using
     **/
    class logger {
        
        public :
        
            enum logstate {
                info  = 0,
                warn  = 1,
                error = 2
            };
        
        
            
            static logger* getInstance( void );
            //void setMessage( const logstate&, const std::string& );
            //void setLogOn( void );
            //void setLogOff( void );
            std::string getFilename( void ) const;  
                            
        
        private : 
        
            /** local instance **/
            static logger* m_instance;
        
            /** filename for logging output **/
            static std::string m_filename;
        
        
            /** empty constructor **/
            logger( void ) {};  
        
            /** empty destructor **/
            ~logger( void ) {}; 
            
            /** empty copy-constructor **/
            logger( const logger& ) {};
        
            /** empty equal operator **/
            logger& operator=( const logger& ) {};
        
    };
    

    
    /** returns the instance
     * @return logger instance
     **/
    inline logger* logger::getInstance()
    {
        if (!m_instance)
            m_instance = new logger;
        
        return m_instance;
    }
    
    
    /** returns the temporary filename for logging
     * @return string with path and filename
     **/
    inline std::string logger::getFilename( void ) const
    {
        return m_filename;
    }
    
    
};};



#endif