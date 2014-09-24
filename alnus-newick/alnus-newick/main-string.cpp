//
//  main-string.cpp
//  alnus-newick
//
//  Created by Sang Chul Choi on 9/12/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

//
//  main.cpp
//  alnus-newick
//
//  Created by Sang Chul Choi on 9/8/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

// References
// http://www.cplusplus.com/
// http://stackoverflow.com/questions/13853774/infix-calculator-expression-parser
// http://en.wikipedia.org/wiki/Extended_Backus–Naur_Form
//
// http://stackoverflow.com/questions/5590381/easiest-way-to-convert-int-to-string-in-c
//
// Not using boost:
// std::string untrimmed( "   This is an untrimmed string!    " );
// std::string trimmed( boost::algorithm::trim( s ) );

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <random>

#include <cctype>
#include <cassert>
#include <cmath>

#pragma mark - classes


class AlnusPhyolgeticBranch
{
public:
    AlnusPhyolgeticBranch(double length = 0, std::string label = "");
    ~AlnusPhyolgeticBranch();
    void addChild(AlnusPhyolgeticBranch *child);
public:
    void setLength(double length);
    void setLabel(std::string label);
    void description();
    void computeLengthToRoot(double lengthToRootOfParent = 0);
    void setLengthToRoot(double lengthToRoot);
    unsigned int getSetNumberOfNodes();
    void setRowIndex(unsigned int baseIndex);
    unsigned int getNumberOfNodes();
    void setCoordinates(double x_max, double x_scale = 100);
    double getMaxLengthToRoot();
    AlnusPhyolgeticBranch* branchAt(size_t i);
    void draw();
    
    void toNewickString(std::string& s);
    
    // Accessors
    double getLengthToRoot() const;
    double getXBegin() const;
    double getXEnd() const;
    double getYBegin() const;
    double getYEnd() const;
    
    bool isInternal() const;
    bool isExternal() const;
    double getXChild() const;
    double getYChildBegin() const;
    double getYChildEnd() const;
    std::string getLabel() const;
private:
    double lengthToRoot();
    
private:
    double mLength;
    std::string mLabel;
    std::vector<AlnusPhyolgeticBranch*> mChildren;
private:
    double mLengthToRoot;
    unsigned int mNumberOfNodes;
    unsigned int mRowIndex;
    double mXBegin;
    double mYBegin;
    double mXEnd;
    double mYEnd;
    double mXScale;
};

class AlnusPhyolgeticTree
{
public:
    AlnusPhyolgeticTree();
    ~AlnusPhyolgeticTree();
    void draw();
    void setRoot(AlnusPhyolgeticBranch *root);
    void setMaxLengthToRoot();
    void description();
    std::string toNewickString();
private:
    
    AlnusPhyolgeticBranch *mRoot;
    unsigned int mNumberOfNodes;
    double mMaxLengthToRoot;
    double mXScale;
    double mYScale;
};

namespace Alnus {
    
    class TreeGenerator
    {
    public:
        static AlnusPhyolgeticTree* generate(double maxLength = 10.0, double length = 1.0, double numberOfChildren = 2.0);
        static AlnusPhyolgeticBranch* sample(double lengthToRoot = 0);
    private:
        static double mMaxLength;
        static double mLength;
        static double mNumberOfChildren;
        static unsigned int mIndex;
        static std::default_random_engine mGenerator;
        static std::poisson_distribution<int> mPoissonDistribution;
        static std::exponential_distribution<double> mExponentialDistribution;
    };
    
    double TreeGenerator::mMaxLength = 10.0;
    double TreeGenerator::mLength = 1.0;
    double TreeGenerator::mNumberOfChildren = 2.0;
    unsigned int TreeGenerator::mIndex = 1;
    std::default_random_engine TreeGenerator::mGenerator;
    std::poisson_distribution<int> TreeGenerator::mPoissonDistribution(TreeGenerator::mNumberOfChildren);
    std::exponential_distribution<double> TreeGenerator::mExponentialDistribution(TreeGenerator::mLength);
    
}



class AlnusNewickParser
{
public:
    AlnusNewickParser(std::string newickTree = "");
    ~AlnusNewickParser();
    AlnusPhyolgeticTree* parse();
    
    
    void setNewickTree(std::string newickTree);
    
    enum class Operator {
        LEFT_PARENTHESIS, RIGHT_PARENTHESIS, QUOTATION_MARK,
        LEFT_BRACKET, RIGHT_BRACKET,
        NUMBER, END, LABEL, COMMA, COLON, SEMICOLON};
private:
    
    AlnusPhyolgeticTree* tree();
    AlnusPhyolgeticBranch* subtree();
    AlnusPhyolgeticBranch* descendants();
    int reset();
    int initialize();
    int next();
    int read();
    int readForQuotedLabel();
    int checkForQuotedLabel();
    int ignore();
    Operator scan();
    bool isLabel(int ch);
    
    
private:
    std::string mNewickTree;
    std::string mBuffer;
    unsigned mBufferPosition;
    int mCharacter;
    size_t mPosition;
    Operator mOperator;
};

class AlnusPoint
{
public:
    AlnusPoint(double x = 0, double y = 0);
    double mX;
    double mY;
};

class AlnusGraphics
{
public:
    AlnusGraphics(size_t x = 100, size_t y = 100);
    void drawHorizontalLine(AlnusPoint begin, AlnusPoint end);
    void drawVerticalLine(AlnusPoint begin, AlnusPoint end);
    void drawText(std::string text, AlnusPoint position);
    void print();
private:
    size_t mX;
    size_t mY;
    std::vector<std::string> mCoordinate;
};

#pragma mark - main

int main(int argc, const char * argv[]) {
    
    std::cout << "argc: " << argc << std::endl;
    
    if (argc == 1) {
        // Use the standard input.
        std::string newickTree("(((One:0.2,Two:0.3):0.3,(Three:0.5,Four:0.3):0.2):0.3,Five:0.7):0.0;");
        std::cout << "Please, type a string in Newick Tree format or just ENTER key: ";
        int ch = std::cin.get();
        if (ch == '\n') {
            // Use the default tree.
        } else {
            std::cin.putback(ch);
            std::string input;
            std::getline(std::cin, input);
            newickTree = input;
        }
        std::cout << "We use the following string in Newick Tree format.";
        std::cout << newickTree << std::endl;
        
        AlnusNewickParser parser(newickTree);
        AlnusPhyolgeticTree *tree = parser.parse();
        tree->description();
        tree->draw();
        delete tree;
    } else if (argc == 2 && !strcmp(argv[1], "-g")) {
        AlnusPhyolgeticTree *t = Alnus::TreeGenerator::generate(1, 1, 2);
        std::cout << "Newick tree: " << t->toNewickString() << std::endl;
        t->description();
        t->draw();
        delete t;
    } else {
        AlnusNewickParser parser;
        // Use the input files.
        for (int i = 1; i < argc; i++) {
            std::cout << argv[i] << std::endl;
            std::ifstream inFile(argv[i]);
            
            std::string newickTree;
            while (std::getline(inFile, newickTree)) {
                std::cout << "Tree: " << newickTree << std::endl;
                
                if (newickTree.length() > 0) {
                    parser.setNewickTree(newickTree);
                    AlnusPhyolgeticTree *tree = parser.parse();
                    tree->description();
                    tree->draw();
                    delete tree;
                }
            }
            
            if (inFile.bad()) {
                // IO error
            } else if (!inFile.eof()) {
                // format error (not possible with getline but possible with operator>>)
            } else {
                // format error (not possible with getline but possible with operator>>)
                // or end of file (can't make the difference)
            }
            
            inFile.close();
        }
    }
    
    return 0;
}

#pragma mark - Utilities

template<class T> class DeleteVector
{
public:
    // Overloaded () operator.
    // This will be called by for_each() function.
    bool operator()(T x) const
    {
        // Delete pointer.
        delete x;
        return true;
    }
};

template<class T> class CallDescriptionVector
{
public:
    // Overloaded () operator.
    // This will be called by for_each() function.
    bool operator()(T x) const
    {
        // Delete pointer.
        x->description();
        return true;
    }
};

//template<class T> class CallLengthToRootVector
//{
//public:
//    // Overloaded () operator.
//    // This will be called by for_each() function.
//    bool operator()(T x) const
//    {
//        // Delete pointer.
//        x->computeLengthToRoot();
//        return true;
//    }
//};

inline std::string trim_right_copy(
                                   const std::string& s,
                                   const std::string& delimiters = " \f\n\r\t\v" )
{
    return s.substr( 0, s.find_last_not_of( delimiters ) + 1 );
}

inline std::string trim_left_copy(
                                  const std::string& s,
                                  const std::string& delimiters = " \f\n\r\t\v" )
{
    return s.substr( s.find_first_not_of( delimiters ) );
}

inline std::string trim_copy(
                             const std::string& s,
                             const std::string& delimiters = " \f\n\r\t\v" )
{
    return trim_left_copy( trim_right_copy( s, delimiters ), delimiters );
}

#pragma mark - Branch


AlnusPhyolgeticBranch::AlnusPhyolgeticBranch(double length,
                                             std::string label)
: mLength(length), mLabel(label)
{
    std::cout << "Branch: created " << std::endl;
}

AlnusPhyolgeticBranch::~AlnusPhyolgeticBranch()
{
    //    std::cout << "Branch: deleted [" << mLabel << "]:" << mLength << std::endl;
    //    mChildren.erase(mChildren.begin(), mChildren.end());
    for_each(mChildren.begin(),
             mChildren.end(),
             DeleteVector<AlnusPhyolgeticBranch*>());
    mChildren.clear();
    std::cout << "Branch: deleted [" << mLabel << "]:" << mLength << std::endl;
}

void AlnusPhyolgeticBranch::setLength(double length)
{
    mLength = length;
}

void AlnusPhyolgeticBranch::setLabel(std::string label)
{
    mLabel = label;
}

void AlnusPhyolgeticBranch::setLengthToRoot(double lengthToRoot)
{
    mLengthToRoot = lengthToRoot;
}

void AlnusPhyolgeticBranch::addChild(AlnusPhyolgeticBranch *child)
{
    mChildren.push_back(child);
}

void AlnusPhyolgeticBranch::description()
{
    //    std::cout << "Branch: [" << mLabel << "]:" << mLength << std::endl;
    for_each(mChildren.begin(),
             mChildren.end(),
             CallDescriptionVector<AlnusPhyolgeticBranch*>());
    std::cout << "Branch: [" << mLabel << "]:" << mLength
    << "(" << mLengthToRoot << ")"
    << "D:(" << mNumberOfNodes << ")"
    << "R:(" << mRowIndex << ")"
    << "Bxy:(" << mXBegin << "," << mYBegin << ")"
    << "Exy:(" << mXEnd << "," << mYEnd << ")"
    << std::endl;
}

void AlnusPhyolgeticBranch::computeLengthToRoot(double lengthToRootOfParent)
{
    mLengthToRoot = mLength + lengthToRootOfParent;
    // goshng: Use for_each?
    //    for_each(mChildren.begin(), mChildren.end(),
    //             std::bind1st(std::mem_fun(&AlnusPhyolgeticBranch::computeLengthToRoot), lengthToRootOfParent));
    for (std::vector<AlnusPhyolgeticBranch*>::iterator i = mChildren.begin();
         i != mChildren.end(); i++) {
        (*i)->computeLengthToRoot(mLengthToRoot);
    }
}

double AlnusPhyolgeticBranch::getMaxLengthToRoot()
{
    double maxLength = mLengthToRoot;
    for (std::vector<AlnusPhyolgeticBranch*>::iterator i = mChildren.begin();
         i != mChildren.end(); i++) {
        double lengthToRoot = (*i)->getMaxLengthToRoot();
        if (maxLength < lengthToRoot) {
            maxLength = lengthToRoot;
        }
    }
    return maxLength;
}

unsigned int
AlnusPhyolgeticBranch::getSetNumberOfNodes()
{
    mNumberOfNodes = 0;
    
    for (std::vector<AlnusPhyolgeticBranch*>::iterator i = mChildren.begin();
         i != mChildren.end(); i++) {
        mNumberOfNodes += (*i)->getSetNumberOfNodes();
    }
    
    return mNumberOfNodes + 1;
}

unsigned int
AlnusPhyolgeticBranch::getNumberOfNodes()
{
    return mNumberOfNodes;
}

void
AlnusPhyolgeticBranch::setRowIndex(unsigned int baseIndex)
{
    if (mChildren.empty()) {
        mRowIndex = baseIndex;
        return;
    }
    //    unsigned int numberOfNodesInTheMostLeftChild = mChildren.front()->getNumberOfNodes() + 1;
    
    // Cumulative numbers of nodes in the reverse order
    std::vector<unsigned int> numbersOfNodesInRightChildren;
    unsigned int numberOfNodes = 0;
    numbersOfNodesInRightChildren.push_back(numberOfNodes);
    for (std::vector<AlnusPhyolgeticBranch*>::reverse_iterator i = mChildren.rbegin();
         i != mChildren.rend(); i++) {
        numberOfNodes += (*i)->getNumberOfNodes() + 1;
        numbersOfNodesInRightChildren.push_back(numberOfNodes);
    }
    
    // Recursive calls
    mRowIndex = baseIndex - numbersOfNodesInRightChildren.at(mChildren.size() - 1);
    //    assert(mRowIndex == numberOfNodesInTheMostLeftChild + 1);
    AlnusPhyolgeticBranch *branch = mChildren.at(0);
    branch->setRowIndex(mRowIndex - 1);
    
    for (size_t i = 1; i < mChildren.size(); i++) {
        branch = mChildren.at(i);
        unsigned int childBaseIndex = baseIndex - numbersOfNodesInRightChildren.at(mChildren.size() - 1 - i);
        branch->setRowIndex(childBaseIndex);
    }
    
}

// Y-scale and Y-max
void AlnusPhyolgeticBranch::setCoordinates(double x_max, double x_scale)
{
    mXScale = x_scale;
    mXEnd = mLengthToRoot / x_max * x_scale;
    mYEnd = mRowIndex;
    mXBegin = mXEnd - (mLength / x_max * x_scale);
    mYBegin = mYEnd;
    
    for (std::vector<AlnusPhyolgeticBranch*>::iterator i = mChildren.begin();
         i != mChildren.end(); i++) {
        (*i)->setCoordinates(x_max, x_scale);
    }
    
}

void AlnusPhyolgeticBranch::draw()
{
    for (int i = 0; i < static_cast<int>(mXScale); i++) {
        if (static_cast<double>(i) < mXBegin || static_cast<double>(i) > mXEnd) {
            std::cout << " ";
        } else {
            std::cout << "-";
        }
    }
    if (mLabel.length() > 0) {
        std::cout << mLabel;
    }
    std::cout << std::endl;
}

AlnusPhyolgeticBranch*
AlnusPhyolgeticBranch::branchAt(size_t indexOfBranch)
{
    if (mRowIndex == indexOfBranch) {
        return this;
    }
    
    for (std::vector<AlnusPhyolgeticBranch*>::iterator i = mChildren.begin();
         i != mChildren.end(); i++) {
        AlnusPhyolgeticBranch *branch = (*i)->branchAt(indexOfBranch);
        if (branch != nullptr) {
            return branch;
        }
    }
    return nullptr;
}

double AlnusPhyolgeticBranch::getLengthToRoot() const
{
    return mLengthToRoot;
}

double AlnusPhyolgeticBranch::getXBegin() const
{
    return mXBegin;
}

double AlnusPhyolgeticBranch::getXEnd() const
{
    return mXEnd;
}

double AlnusPhyolgeticBranch::getYBegin() const
{
    return mYBegin;
}

double AlnusPhyolgeticBranch::getYEnd() const
{
    return mYEnd;
}

bool AlnusPhyolgeticBranch::isInternal() const
{
    if (mChildren.size() == 0) {
        return false;
    } else {
        return true;
    }
}

bool AlnusPhyolgeticBranch::isExternal() const
{
    return !isInternal();
}

double AlnusPhyolgeticBranch::getXChild() const
{
    return getXEnd();
}

double AlnusPhyolgeticBranch::getYChildBegin() const
{
    return mChildren.front()->getYBegin();
}

double AlnusPhyolgeticBranch::getYChildEnd() const
{
    return mChildren.back()->getYBegin();
}

std::string AlnusPhyolgeticBranch::getLabel() const
{
    return mLabel;
}

void
AlnusPhyolgeticBranch::toNewickString(std::string& s)
{
    if (mChildren.size() > 0) {
        s.append("(");
        for (std::vector<AlnusPhyolgeticBranch*>::iterator i = mChildren.begin();
             i != mChildren.end(); i++) {
            if (i != mChildren.begin()) {
                s.append(",");
            }
            (*i)->toNewickString(s);
        }
        s.append(")");
        if (mLabel.size() > 0) {
            s.append(mLabel);
        }
    } else {
        s.append(mLabel);
    }
    
    if (mLength > 0) {
        s.append(":");
        std::string stringLength = std::to_string(mLength);
        s.append(stringLength);
    }
}

#pragma mark - Tree

AlnusPhyolgeticTree::AlnusPhyolgeticTree()
{
    std::cout << "Tree: created." << std::endl;
}

AlnusPhyolgeticTree::~AlnusPhyolgeticTree()
{
    delete mRoot;
    std::cout << "Tree: deleted." << std::endl;
}

void AlnusPhyolgeticTree::draw()
{
    
    AlnusGraphics graphics(mXScale, mYScale + 1);
    
    for (size_t i = 1; i <= mNumberOfNodes; i++) {
        AlnusPhyolgeticBranch *branch = mRoot->branchAt(i);
        
        // Draw!
        //        AlnusPoint
        
        AlnusPoint begin(branch->getXBegin(), branch->getYBegin());
        AlnusPoint end(branch->getXEnd(), branch->getYEnd());
        graphics.drawHorizontalLine(begin, end);
    }
    
    for (size_t i = 1; i <= mNumberOfNodes; i++) {
        AlnusPhyolgeticBranch *branch = mRoot->branchAt(i);
        
        if (branch->isInternal()) {
            AlnusPoint begin(branch->getXChild(), branch->getYChildBegin());
            AlnusPoint end(branch->getXChild(), branch->getYChildEnd());
            graphics.drawVerticalLine(begin, end);
        }
    }
    
    for (size_t i = 1; i <= mNumberOfNodes; i++) {
        AlnusPhyolgeticBranch *branch = mRoot->branchAt(i);
        
        if (branch->isExternal()) {
            AlnusPoint begin(branch->getXEnd(), branch->getYEnd());
            graphics.drawText(branch->getLabel(), begin);
        }
    }
    
    
    
    graphics.print();
}


void AlnusPhyolgeticTree::setRoot(AlnusPhyolgeticBranch *root)
{
    mRoot = root;
}

void AlnusPhyolgeticTree::setMaxLengthToRoot()
{
    mMaxLengthToRoot = mRoot->getMaxLengthToRoot();
}

void AlnusPhyolgeticTree::description()
{
    mXScale = 50;
    mRoot->computeLengthToRoot();
    setMaxLengthToRoot();
    mNumberOfNodes = mRoot->getSetNumberOfNodes();
    std::cout << "Tree: Nodes(" << mNumberOfNodes << ")" << std::endl;
    mRoot->setRowIndex(mNumberOfNodes);
    mRoot->setCoordinates(mMaxLengthToRoot, mXScale);
    mYScale = static_cast<double>(mNumberOfNodes);
    mRoot->description();
}

std::string
AlnusPhyolgeticTree::toNewickString()
{
    std::string s;
    mRoot->toNewickString(s);
    s.append(";");
    return s;
}

#pragma mark - Generator

AlnusPhyolgeticTree*
Alnus::TreeGenerator::generate(double maxLength, double length, double numberOfChildren)
{
    mIndex = 1;
    mMaxLength = maxLength;
    mLength = length;
    mNumberOfChildren = numberOfChildren;
    AlnusPhyolgeticTree *t = new AlnusPhyolgeticTree;
    t->setRoot(sample());
    return t;
}

AlnusPhyolgeticBranch*
Alnus::TreeGenerator::sample(double lengthToRoot)
{
    AlnusPhyolgeticBranch *branch = new AlnusPhyolgeticBranch;
    double length = mExponentialDistribution(mGenerator);
    //    int numberOfChildren = mPoissonDistribution(mGenerator);
    int numberOfChildren = 2;
    branch->setLength(length);
    branch->setLengthToRoot(length + lengthToRoot);
    if (branch->getLengthToRoot() < mMaxLength) {
        
        for (int i = 0; i < numberOfChildren; i++) {
            AlnusPhyolgeticBranch *childBranch = sample(branch->getLengthToRoot());
            branch->addChild(childBranch);
        }
    } else {
        std::string label = std::to_string(mIndex++);
        branch->setLabel(label);
    }
    return branch;
}
#pragma mark - NewickParser

AlnusNewickParser::AlnusNewickParser(std::string newickTree)
{
    std::cout << "Parser: created." << std::endl;
    if (newickTree.length() > 0) {
        setNewickTree(newickTree);
    }
}

AlnusNewickParser::~AlnusNewickParser()
{
    std::cout << "Parser: deleted." << std::endl;
}

void AlnusNewickParser::setNewickTree(std::string newickTree)
{
    mNewickTree = newickTree;
    
    mPosition = 0;
    initialize();
}

AlnusPhyolgeticTree* AlnusNewickParser::parse()
{
    AlnusPhyolgeticTree *t = tree();
    AlnusPhyolgeticBranch *root = subtree();
    t->setRoot(root);
    return t;
}

AlnusPhyolgeticTree*
AlnusNewickParser::tree()
{
    AlnusPhyolgeticTree *tree = new AlnusPhyolgeticTree;
    
    return tree;
}

AlnusPhyolgeticBranch*
AlnusNewickParser::subtree()
{
    AlnusPhyolgeticBranch *branch = nullptr;
    
    switch (mOperator) {
        case Operator::LEFT_PARENTHESIS:
            next();
            branch = descendants();
            if (mOperator != Operator::RIGHT_PARENTHESIS) {
                // Exception?
            }
            next();
            if (mOperator == Operator::LABEL) {
                // label of a branch.
                branch->setLabel(mBuffer);
            }
            break;
        case Operator::LABEL:
            // label of a leaf.
            branch = new AlnusPhyolgeticBranch;
            branch->setLabel(mBuffer);
            next();
            break;
        default:
            // Exception?
            assert(0);
            break;
    }
    
    if (mOperator == Operator::COLON) {
        next();
        assert(mOperator == Operator::NUMBER);
        // branch length.
        double length = std::stod(mBuffer);
        branch->setLength(length);
        next();
    }
    return branch;
}

AlnusPhyolgeticBranch*
AlnusNewickParser::descendants()
{
    AlnusPhyolgeticBranch *branch = new AlnusPhyolgeticBranch;
    AlnusPhyolgeticBranch *childBranch = subtree();
    branch->addChild(childBranch);
    while (mOperator == Operator::COMMA) {
        next();
        childBranch = subtree();
        branch->addChild(childBranch);
    }
    return branch;
}

int AlnusNewickParser::initialize()
{
    reset();
    ignore();
    next();
    return 0;
}

int AlnusNewickParser::reset()
{
    mBuffer.clear();
    return 0;
}

int AlnusNewickParser::next()
{
    mOperator = scan();
    return 0;
}

int AlnusNewickParser::read()
{
    mBuffer += mCharacter;
    mCharacter = mNewickTree.at(mPosition++); // same in ignore?
    return 0;
}

int AlnusNewickParser::readForQuotedLabel()
{
    mBuffer += mCharacter;
    checkForQuotedLabel();
    return 0;
}

// Requirements: Single quote characters in a quoted label are represented by two single
// quotes.
int AlnusNewickParser::checkForQuotedLabel()
{
    int firstCharacter = mNewickTree.at(mPosition);
    int secondCharacter = mNewickTree.at(mPosition + 1);
    // goshng: check the out-of-bound
    if (firstCharacter == '\'' && secondCharacter == '\'') {
        mCharacter = '\'';
        mPosition += 2;
    } else {
        if (firstCharacter == '\'') {
            mCharacter = 0;
        } else {
            mCharacter = firstCharacter;
        }
        mPosition++;
    }
    return 0;
}

int AlnusNewickParser::ignore()
{
    mCharacter = mNewickTree.at(mPosition++);
    return 0;
}

AlnusNewickParser::Operator
AlnusNewickParser::scan()
{
INIT:
    reset();
    
    if (mCharacter == ' ') {
        ignore();
    }
    
    if (mOperator == Operator::COLON) {
        read();
        goto LEADING_DIGIT;
    }
    //    if (mOperator == Operator::QUOTATION_MARK) {
    //        read();
    //        goto ALPHABET;
    //    }
    
    if (mCharacter == '\'') {
        goto ALPHABET_WITH_QUOTATION_MARK;
    }
    
    if (isLabel(mCharacter)) {
        read();
        goto ALPHABET;
    }
    
    //    if (isnumber(mCharacter)) {
    //        read();
    //        goto LEADING_DIGIT;
    //    }
START:
    switch (mCharacter) {
            // Requirements: Blanks or tabs may appear anywhere except within unquoted labels or
            // branch_lengths.
        case ' ':
        case '\t':
        case '\r':
        case '\n':
            ignore();
            goto START;
            break;
        case ',':
            read();
            return Operator::COMMA;
        case '(':
            read();
            return Operator::LEFT_PARENTHESIS;
        case ')':
            read();
            return Operator::RIGHT_PARENTHESIS;
        case '[':
            while (mCharacter != ']') {
                read();
            }
            goto START;
        case ']':
            //            reset();
            read();
            goto INIT;
        case ':':
            mCharacter = ' ';
            return Operator::COLON;
        case ';':
            mCharacter = ' ';
            return Operator::SEMICOLON;
            //        case '\n':
            //            mCharacter = ' ';
            //            return Operator::END;
        default:
            // Exception?
            assert(0);
            break;
    }
    
LEADING_DIGIT:
    if (mCharacter == '-' || mCharacter == '+') {
        read();
    }
    if (isnumber(mCharacter)) {
        read();
        goto LEADING_DIGIT;
    } else if (mCharacter == '.') {
        read();
        goto TRAILING_DIGIT;
    } else {
        return Operator::NUMBER;
    }
TRAILING_DIGIT:
    if (isnumber(mCharacter)) {
        read();
        goto TRAILING_DIGIT;
    } else {
        return Operator::NUMBER;
    }
    
ALPHABET:
    while (isLabel(mCharacter)) {
        // Requirements: Underscore characters in unquoted labels are converted to blanks.
        if (mCharacter == '_') {
            mCharacter = ' ';
        }
        read();
    }
    return Operator::LABEL;
ALPHABET_WITH_QUOTATION_MARK:
    checkForQuotedLabel();
    while (mCharacter != 0) {
        readForQuotedLabel();
    }
    // Trim the string.
    mBuffer = trim_copy(mBuffer);
    return Operator::LABEL;
}

// Requirements: Unquoted labels may not contain blanks, parentheses, square brackets,
// single_quotes, colons, semicolons, or commas.
bool
AlnusNewickParser::isLabel(int ch)
{
    if (isspace(ch)) {
        return false;
    }
    switch (ch) {
        case '(':
        case ')':
        case '[':
        case ']':
        case '\'':
        case ':':
        case ';':
        case ',':
            return false;
            break;
        default:
            return true;
            break;
    }
}

#pragma mark - Graphics

AlnusPoint::AlnusPoint(double x, double y)
: mX(x), mY(y)
{
    
}

AlnusGraphics::AlnusGraphics(size_t x, size_t y)
: mX(x), mY(y)
{
    mCoordinate.resize(y);
    for (std::vector<std::string>::iterator i = mCoordinate.begin(); i != mCoordinate.end(); i++) {
        i->assign(x, ' ');
    }
}

void AlnusGraphics::drawHorizontalLine(AlnusPoint begin, AlnusPoint end)
{
    
    size_t xBegin = static_cast<size_t>(std::round(begin.mX));
    size_t xEnd = static_cast<size_t>(std::round(end.mX));
    size_t yBegin = static_cast<size_t>(begin.mY);
    size_t yEnd = static_cast<size_t>(end.mY);
    std::string &line = mCoordinate.at(yBegin);
    for (size_t i = 0; i < line.size(); i++) {
        if (xBegin <= i && i <= xEnd) {
            line.at(i) = '-';
        }
    }
}


void AlnusGraphics::drawVerticalLine(AlnusPoint begin, AlnusPoint end)
{
    size_t xBegin = static_cast<size_t>(std::round(begin.mX));
    size_t xEnd = static_cast<size_t>(end.mX);
    size_t yBegin = static_cast<size_t>(begin.mY);
    size_t yEnd = static_cast<size_t>(end.mY);
    
    for (size_t i = 0; i < mCoordinate.size(); i++) {
        if (yBegin <= i && i <= yEnd) {
            mCoordinate.at(i).at(xBegin) = '|';
        }
        if (yBegin == i) {
            mCoordinate.at(i).at(xBegin) = '/';
        }
        if (yEnd == i) {
            mCoordinate.at(i).at(xBegin) = '\\';
        }
    }
}

void AlnusGraphics::drawText(std::string text, AlnusPoint begin)
{
    size_t xBegin = static_cast<size_t>(begin.mX);
    size_t yBegin = static_cast<size_t>(begin.mY);
    
    text.insert(0, 1, ' ');
    
    std::string &line = mCoordinate.at(yBegin);
    line.replace(xBegin, text.size(), text);
}

void AlnusGraphics::print()
{
    for (std::vector<std::string>::iterator i = mCoordinate.begin(); i != mCoordinate.end(); i++) {
        std::cout << *i << std::endl;
    }
}



















































