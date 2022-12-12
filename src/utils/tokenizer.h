/*
* Copyright 2022 SCRAP
*
* This file is part of Scrappy Tablebase Generator.
* 
* Scrappy Tablebase Generator is free software: you can redistribute it and/or modify it under the terms 
* of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, 
* or (at your option) any later version.
* 
* Scrappy Tablebase Generator is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with Scrappy Tablebase Generator. If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <sstream>

// Provides an interface for other tokenizers to use.
// A child of this class only has to implement next().
class AbstractStreamTokenizer {
    protected:
        std::istream& m_stream;
        std::string m_peeked;
        bool m_hasPeeked = false;
    public:
        AbstractStreamTokenizer(std::istream& _stream) : m_stream(_stream) { };

        // gets the next lexeme and pops it
        virtual std::string next() = 0;

        // gets the next lexeme without popping it
        std::string peek() {
            if (!m_hasPeeked) {
                m_peeked = next();
                m_hasPeeked = true;
            }
            return m_peeked;
        }

        // returns if there are no more lexemes.
        // Override this if the provided code does not work for your tokenizer
        virtual bool hasNext() {
            return m_hasPeeked || m_stream.peek() != EOF;
        }
};

// Provides an interface for other tokenizers to use.
// A child of this class only has to implement next().
class StringTokenizer : public AbstractStreamTokenizer {
    protected:
        std::istringstream m_stream;
    public:
        StringTokenizer(std::string _string) : m_stream(_string), 
        AbstractStreamTokenizer(m_stream) { };
};

#endif