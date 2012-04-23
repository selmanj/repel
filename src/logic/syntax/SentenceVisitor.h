/*
 * SentenceVisitor.h
 *
 *  Created on: May 20, 2011
 *      Author: joe
 */

#ifndef SENTENCEVISITOR_H_
#define SENTENCEVISITOR_H_

class Sentence;

class SentenceVisitor {
public:
    virtual void accept(const Sentence& s) = 0;
    virtual ~SentenceVisitor() {};
};

#endif /* SENTENCEVISITOR_H_ */
