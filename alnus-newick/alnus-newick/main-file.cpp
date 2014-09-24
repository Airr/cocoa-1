//
//  main-file.cpp
//  alnus-newick
//
//  Created by Sang Chul Choi on 9/13/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

#include <stdio.h>

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
//
// http://en.cppreference.com/w/cpp/types/is_pod
// http://blog.smartbear.com/c-plus-plus/the-biggest-changes-in-c11-and-why-you-should-care/
//
// C++11
// http://www.codeproject.com/Articles/570638/Ten-Cplusplus-Features-Every-Cplusplus-Developer
//
// Tricks
// http://stackoverflow.com/questions/8572991/how-to-write-the-range-based-for-loop-with-argv
//



#define DEBUG_LOG
//#undef DEBUG_LOG

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <random>
#include <stdexcept>
#include <map>

#include <cctype>
#include <cassert>
#include <cmath>

#pragma mark - classes

namespace Alnus {
    
    class ParsingException : public std::runtime_error
    {
    public:
        ParsingException();
        
    };
    
    class PhyolgeticBranch
    {
    public:
        PhyolgeticBranch(double length = 0, std::string label = "");
        ~PhyolgeticBranch();
        void addChild(PhyolgeticBranch *child);
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
        PhyolgeticBranch* branchAt(size_t i);
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
        bool isDendrogram() const;
        void setIsDendrogram(bool isDendrogram);
        void setLengthForDendrogram();
        
        static size_t getReferenceCount();
        
        static const double mLengthForDendrogram;
    private:
        double lengthToRoot();
        
    private:
        double mLength;
        std::string mLabel;
        std::vector<PhyolgeticBranch*> mChildren;
    private:
        double mLengthToRoot;
        unsigned int mNumberOfNodes;
        unsigned int mRowIndex;
        double mXBegin;
        double mYBegin;
        double mXEnd;
        double mYEnd;
        double mXScale;
        bool mIsDendrogram;
        static size_t mReferenceCount;
    };
    
    size_t PhyolgeticBranch::mReferenceCount = 0;
    const double PhyolgeticBranch::mLengthForDendrogram = 1.0;
    
    class PhyolgeticTree
    {
    public:
        PhyolgeticTree();
        ~PhyolgeticTree();
        void draw();
        int configure(double xScale = 50);
        void setRoot(PhyolgeticBranch *root);
        void setMaxLengthToRoot();
        void description();
        std::string toNewickString();
    private:
        
        PhyolgeticBranch *mRoot;
        unsigned int mNumberOfNodes;
        double mMaxLengthToRoot;
        double mXScale;
        double mYScale;
    };
    
    
    class TreeGenerator
    {
    public:
        static PhyolgeticTree* generate(double maxLength = 10.0, double length = 1.0, double numberOfChildren = 2.0);
        static PhyolgeticBranch* sample(double lengthToRoot = 0);
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
    
    
    
    
    class NewickParser
    {
    public:
        NewickParser(std::string newickTree = "");
        NewickParser(std::istream *in);
        ~NewickParser();
        PhyolgeticTree* parse();
        PhyolgeticTree* nextTree();
        
        int skip();
        
        std::string getInput() const;
        void setNewickTree(std::string newickTree);
        
        enum class Operator {
            LEFT_PARENTHESIS, RIGHT_PARENTHESIS, QUOTATION_MARK,
            LEFT_BRACKET, RIGHT_BRACKET,
            NUMBER, END_OF_FILE, END_OF_LINE,
            LABEL, COMMA, COLON, SEMICOLON,
            ERROR_NO_LABEL, UNKNOWN
        };
        
        void printErrorMessage();
        
        enum class ErrorCode {
            okay,
            memory,
            number,
            twoLabels,
            emptyDescendant,
            noLeftParenthesis,
            noMatchingParentheses,
            labelFollower,
            subtree,
            noNumberAfterColon
        };
        
        static const std::map<ErrorCode, const char*>::value_type mErrorTable[];
        static const std::map<ErrorCode, const char*> mErrorCodeTable;
        
        
    private:
        
        PhyolgeticTree* tree();
        PhyolgeticBranch* subtree();
        PhyolgeticBranch* descendants();
        int reset();
        bool initialize();
        bool readFirstLeftParenthesis();
        int next();
        int read();
        int readForQuotedLabel();
        int checkForQuotedLabel();
        int ignore();
        Operator scan();
        bool isLabel(int ch);
        bool isLeadingNumber(int ch);
        int putback();
        PhyolgeticBranch* label(PhyolgeticBranch *branch);
        
    private:
        ErrorCode mErrorCode;
        std::string mInput;
        std::istream *mIn;
        std::string mNewickTree;
        std::string mBuffer;
        unsigned mBufferPosition;
        int mCharacter;
        size_t mPosition;
        Operator mOperator;
    };
    
    const std::map<NewickParser::ErrorCode, const char*>::value_type NewickParser::mErrorTable[] = {
        { NewickParser::ErrorCode::okay, "ERR_OK" },
        { NewickParser::ErrorCode::number, "Comma, right parenthesis, semi-colon must follow a number." },
        { NewickParser::ErrorCode::twoLabels, "Two unquoted labels." },
        { NewickParser::ErrorCode::emptyDescendant, "No descendants within parentheses." },
        { NewickParser::ErrorCode::noLeftParenthesis, "No left parenthesis." },
        { NewickParser::ErrorCode::noMatchingParentheses, "No matching parentheses." },
        { NewickParser::ErrorCode::labelFollower, "Colon, comma, semi-colon, right-parenthesis, or left bracket must follow a label." },
        { NewickParser::ErrorCode::subtree, "Subtree must be either descendant list or a leaf label." },
        { NewickParser::ErrorCode::noNumberAfterColon, "Number must follow a colon." },
        { NewickParser::ErrorCode::memory, "Not enough memory" }
    };
    
    const std::map<NewickParser::ErrorCode, const char*> NewickParser::mErrorCodeTable(begin(NewickParser::mErrorTable),end(NewickParser::mErrorTable));
    
    
    class Point
    {
    public:
        Point(double x = 0, double y = 0);
        double mX;
        double mY;
    };
    
    class Graphics
    {
    public:
        Graphics(size_t x = 100, size_t y = 100);
        void drawHorizontalLine(Point begin, Point end, bool isDendrogram = false);
        void drawVerticalLine(Point begin, Point end);
        void drawPoint(Point end);
        void drawText(std::string text, Point position);
        void print();
    private:
        size_t mX;
        size_t mY;
        std::vector<std::string> mCoordinate;
    };
    
}

#pragma mark - main

void mainForNewickFile(std::istream &inFile)
{
    Alnus::NewickParser parser(&inFile);
    Alnus::PhyolgeticTree *t = nullptr;
    
    while (!inFile.eof()) {
        
        try {
            t = parser.nextTree();
        } catch (Alnus::ParsingException &unknownExeption) {
            parser.skip();
            std::cout << "Input: [" << parser.getInput() << "]" << std::endl;
            //                    std::cout << "Exception occurred: " << unknownExeption.what() << std::endl;
            parser.printErrorMessage();
            std::cout << std::endl;
            assert(Alnus::PhyolgeticBranch::getReferenceCount() == 0);
            continue;
        }
        
        if (inFile.eof()) {
            continue;
        }
        if (inFile.bad()) {
            break;
        }
        
        
        
        if (t == nullptr) {
            std::cout << "Input: [" << parser.getInput() << "]" << std::endl;
            std::cout << "Error occurred: " << std::endl;
            parser.printErrorMessage();
            std::cout << std::endl;
            assert(Alnus::PhyolgeticBranch::getReferenceCount() == 0);
            continue;
        }
        
        //                std::cout << std::endl << "Input: [" << parser.getInput() << "]" << std::endl;
        std::cout << "Input: [";
        std::cout << parser.getInput();
        std::cout << "]";
        std::cout << std::endl;
#if defined(DEBUG_LOG)
        t->description();
#endif
        t->draw();
        delete t;
        std::cout << std::endl;
        assert(Alnus::PhyolgeticBranch::getReferenceCount() == 0);
    }
    
    
    if (inFile.eof()) {
#if defined(DEBUG_LOG)
        std::cout << "Reached the end of file!" << std::endl;
#endif
    }
    
    if (inFile.bad()) {
        // IO error
    } else if (!inFile.eof()) {
        // format error (not possible with getline but possible with operator>>)
        
    } else {
        // format error (not possible with getline but possible with operator>>)
        // or end of file (can't make the difference)
    }
}

int main(int argc, const char * argv[]) {
    
    
    if (argc == 1) {
        // Use the standard input.
        std::string newickTree("(((One:0.2,Two:0.3):0.3,(Three:0.5,Four:0.3):0.2):0.3,Five:0.7):0.0;");
        std::cout << "Please, type a string in Newick Tree format and ENTER key: ";
        
        Alnus::NewickParser parser(&std::cin);
        
        while (Alnus::PhyolgeticTree *t = parser.nextTree()) {
            t->description();
            t->draw();
            delete t;
        }
        
    } else if (argc == 2 && !strcmp(argv[1], "-g")) {
        Alnus::PhyolgeticTree *t = Alnus::TreeGenerator::generate(1, 1, 2);
        std::cout << "Newick tree: " << t->toNewickString() << std::endl;
        t->description();
        t->draw();
        delete t;
    } else {
        std::vector<std::string> arguments(argv + 1, argv + argc);
        for (auto &s : arguments) {
            //
            //            std::cout << s << std::endl;
            std::ifstream inFile(s);
            //            inFile.close();
            //        }
            //        for (int i = 1; i < argc; i++) {
            //            std::ifstream inFile(argv[i]);
            
            Alnus::NewickParser parser(&inFile);
            Alnus::PhyolgeticTree *t = nullptr;
            
            while (!inFile.eof()) {
                
                try {
                    t = parser.nextTree();
                } catch (Alnus::ParsingException &unknownExeption) {
                    parser.skip();
                    std::cout << "Input: [" << parser.getInput() << "]" << std::endl;
                    //                    std::cout << "Exception occurred: " << unknownExeption.what() << std::endl;
                    parser.printErrorMessage();
                    std::cout << std::endl;
                    assert(Alnus::PhyolgeticBranch::getReferenceCount() == 0);
                    continue;
                }
                
                if (inFile.eof()) {
                    continue;
                }
                if (inFile.bad()) {
                    break;
                }
                
                
                
                if (t == nullptr) {
                    std::cout << "Input: [" << parser.getInput() << "]" << std::endl;
                    std::cout << "Error occurred: " << std::endl;
                    parser.printErrorMessage();
                    std::cout << std::endl;
                    assert(Alnus::PhyolgeticBranch::getReferenceCount() == 0);
                    continue;
                }
                
                //                std::cout << std::endl << "Input: [" << parser.getInput() << "]" << std::endl;
                std::cout << "Input: [";
                std::cout << parser.getInput();
                std::cout << "]";
                std::cout << std::endl;
#if defined(DEBUG_LOG)
                t->description();
#endif
                t->draw();
                delete t;
                std::cout << std::endl;
                assert(Alnus::PhyolgeticBranch::getReferenceCount() == 0);
            }
            
            
            if (inFile.eof()) {
#if defined(DEBUG_LOG)
                std::cout << "Reached the end of file!" << std::endl;
#endif
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

#define ERR_OK 0
#define ERR_RT_OUT_OF_MEMORY 1
#define ERR_NUMBER 2

typedef std::map<int, const char*> error_code_tbl_t;
typedef error_code_tbl_t::value_type error_code_entry_t;
const error_code_entry_t error_code_tbl_[] = {
    { ERR_OK              , "ERR_OK" },
    { ERR_NUMBER              , "Comma, right parenthesis, semi-colon must follow a number." },
    { ERR_RT_OUT_OF_MEMORY, "ERR_RT_OUT_OF_MEMORY" }
};
const error_code_tbl_t error_code_tbl( begin(error_code_tbl_)
                                      ,end  (error_code_tbl_) );

const char* err2msg(int code)
{
    const error_code_tbl_t::const_iterator it = error_code_tbl.find(code);
    if(it == error_code_tbl.end())
        return "unknown";
    return it->second;
}

namespace Alnus
{
    
#pragma mark - Exception
    
    ParsingException::ParsingException()
    : std::runtime_error("Unknown parsing error")
    {
        
    }
    
    
#pragma mark - Branch
    
    PhyolgeticBranch::PhyolgeticBranch(double length,
                                       std::string label)
    : mLength(length), mLabel(label), mIsDendrogram(false)
    {
#if defined(DEBUG_LOG)
        std::cout << "Branch: created " << std::endl;
#endif
        mReferenceCount++;
    }
    
    PhyolgeticBranch::~PhyolgeticBranch()
    {
        //    std::cout << "Branch: deleted [" << mLabel << "]:" << mLength << std::endl;
        //    mChildren.erase(mChildren.begin(), mChildren.end());
        for_each(mChildren.begin(),
                 mChildren.end(),
                 DeleteVector<PhyolgeticBranch*>());
        mChildren.clear();
#if defined(DEBUG_LOG)
        std::cout << "Branch: deleted [" << mLabel << "]:" << mLength << std::endl;
#endif
        mReferenceCount--;
    }
    
    size_t PhyolgeticBranch::getReferenceCount()
    {
        return mReferenceCount;
    }
    
    void PhyolgeticBranch::setLength(double length)
    {
        mLength = length;
    }
    
    void PhyolgeticBranch::setLabel(std::string label)
    {
        mLabel = label;
    }
    
    void PhyolgeticBranch::setLengthToRoot(double lengthToRoot)
    {
        mLengthToRoot = lengthToRoot;
    }
    
    void PhyolgeticBranch::addChild(PhyolgeticBranch *child)
    {
        mChildren.push_back(child);
    }
    
    void PhyolgeticBranch::description()
    {
        //    std::cout << "Branch: [" << mLabel << "]:" << mLength << std::endl;
        for_each(mChildren.begin(),
                 mChildren.end(),
                 CallDescriptionVector<PhyolgeticBranch*>());
        std::cout << "Branch: [" << mLabel << "]:" << mLength
        << "(" << mLengthToRoot << ")"
        << "D:(" << mNumberOfNodes << ")"
        << "R:(" << mRowIndex << ")"
        << "Bxy:(" << mXBegin << "," << mYBegin << ")"
        << "Exy:(" << mXEnd << "," << mYEnd << ")"
        << std::endl;
    }
    
    void PhyolgeticBranch::computeLengthToRoot(double lengthToRootOfParent)
    {
        mLengthToRoot = mLength + lengthToRootOfParent;
        // goshng: Use for_each?
        //    for_each(mChildren.begin(), mChildren.end(),
        //             std::bind1st(std::mem_fun(&PhyolgeticBranch::computeLengthToRoot), lengthToRootOfParent));
        for (std::vector<PhyolgeticBranch*>::iterator i = mChildren.begin();
             i != mChildren.end(); i++) {
            (*i)->computeLengthToRoot(mLengthToRoot);
        }
    }
    
    double PhyolgeticBranch::getMaxLengthToRoot()
    {
        double maxLength = mLengthToRoot;
        for (std::vector<PhyolgeticBranch*>::iterator i = mChildren.begin();
             i != mChildren.end(); i++) {
            double lengthToRoot = (*i)->getMaxLengthToRoot();
            if (maxLength < lengthToRoot) {
                maxLength = lengthToRoot;
            }
        }
        return maxLength;
    }
    
    unsigned int
    PhyolgeticBranch::getSetNumberOfNodes()
    {
        mNumberOfNodes = 0;
        
        for (std::vector<PhyolgeticBranch*>::iterator i = mChildren.begin();
             i != mChildren.end(); i++) {
            mNumberOfNodes += (*i)->getSetNumberOfNodes();
        }
        
        return mNumberOfNodes + 1;
    }
    
    unsigned int
    PhyolgeticBranch::getNumberOfNodes()
    {
        return mNumberOfNodes;
    }
    
    void
    PhyolgeticBranch::setRowIndex(unsigned int baseIndex)
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
        for (std::vector<PhyolgeticBranch*>::reverse_iterator i = mChildren.rbegin();
             i != mChildren.rend(); i++) {
            numberOfNodes += (*i)->getNumberOfNodes() + 1;
            numbersOfNodesInRightChildren.push_back(numberOfNodes);
        }
        
        // Recursive calls
        mRowIndex = baseIndex - numbersOfNodesInRightChildren.at(mChildren.size() - 1);
        //    assert(mRowIndex == numberOfNodesInTheMostLeftChild + 1);
        PhyolgeticBranch *branch = mChildren.at(0);
        branch->setRowIndex(mRowIndex - 1);
        
        for (size_t i = 1; i < mChildren.size(); i++) {
            branch = mChildren.at(i);
            unsigned int childBaseIndex = baseIndex - numbersOfNodesInRightChildren.at(mChildren.size() - 1 - i);
            branch->setRowIndex(childBaseIndex);
        }
        
    }
    
    // Y-scale and Y-max
    void PhyolgeticBranch::setCoordinates(double x_max, double x_scale)
    {
        mXScale = x_scale;
        mXEnd = mLengthToRoot / x_max * x_scale;
        mYEnd = mRowIndex;
        mXBegin = mXEnd - (mLength / x_max * x_scale);
        mYBegin = mYEnd;
        
        for (std::vector<PhyolgeticBranch*>::iterator i = mChildren.begin();
             i != mChildren.end(); i++) {
            (*i)->setCoordinates(x_max, x_scale);
        }
        
    }
    
    void PhyolgeticBranch::draw()
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
    
    PhyolgeticBranch*
    PhyolgeticBranch::branchAt(size_t indexOfBranch)
    {
        if (mRowIndex == indexOfBranch) {
            return this;
        }
        
        for (std::vector<PhyolgeticBranch*>::iterator i = mChildren.begin();
             i != mChildren.end(); i++) {
            PhyolgeticBranch *branch = (*i)->branchAt(indexOfBranch);
            if (branch != nullptr) {
                return branch;
            }
        }
        return nullptr;
    }
    
    double PhyolgeticBranch::getLengthToRoot() const
    {
        return mLengthToRoot;
    }
    
    double PhyolgeticBranch::getXBegin() const
    {
        return mXBegin;
    }
    
    double PhyolgeticBranch::getXEnd() const
    {
        return mXEnd;
    }
    
    double PhyolgeticBranch::getYBegin() const
    {
        return mYBegin;
    }
    
    double PhyolgeticBranch::getYEnd() const
    {
        return mYEnd;
    }
    
    bool PhyolgeticBranch::isInternal() const
    {
        if (mChildren.size() == 0) {
            return false;
        } else {
            return true;
        }
    }
    
    bool PhyolgeticBranch::isExternal() const
    {
        return !isInternal();
    }
    
    double PhyolgeticBranch::getXChild() const
    {
        return getXEnd();
    }
    
    double PhyolgeticBranch::getYChildBegin() const
    {
        return mChildren.front()->getYBegin();
    }
    
    double PhyolgeticBranch::getYChildEnd() const
    {
        return mChildren.back()->getYBegin();
    }
    
    std::string PhyolgeticBranch::getLabel() const
    {
        return mLabel;
    }
    
    bool PhyolgeticBranch::isDendrogram() const
    {
        return mIsDendrogram;
    }
    
    void PhyolgeticBranch::setIsDendrogram(bool isDendrogram)
    {
        mIsDendrogram = isDendrogram;
    }
    
    void
    PhyolgeticBranch::toNewickString(std::string& s)
    {
        if (mChildren.size() > 0) {
            s.append("(");
            for (std::vector<PhyolgeticBranch*>::iterator i = mChildren.begin();
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
        
        if (mIsDendrogram == false && mLength > 0) {
            s.append(":");
            std::string stringLength = std::to_string(mLength);
            s.append(stringLength);
        }
    }
    
    void
    PhyolgeticBranch::setLengthForDendrogram()
    {
        setIsDendrogram(true);
        setLength(mLengthForDendrogram);
    }
    
#pragma mark - Tree
    
    PhyolgeticTree::PhyolgeticTree()
    : mRoot(nullptr)
    {
#if defined(DEBUG_LOG)
        std::cout << "Tree: created." << std::endl;
#endif
    }
    
    PhyolgeticTree::~PhyolgeticTree()
    {
        if (mRoot == nullptr) {
            //
        } else {
            delete mRoot;
        }
#if defined(DEBUG_LOG)
        std::cout << "Tree: deleted." << std::endl;
#endif
    }
    
    void PhyolgeticTree::draw()
    {
        configure();
        
        Graphics graphics(mXScale, mYScale + 1);
        
        for (size_t i = 1; i <= mNumberOfNodes; i++) {
            PhyolgeticBranch *branch = mRoot->branchAt(i);
            
            // Draw!
            //        Point
            
            Point begin(branch->getXBegin(), branch->getYBegin());
            Point end(branch->getXEnd(), branch->getYEnd());
            bool isDendrogram = branch->isDendrogram();
            graphics.drawHorizontalLine(begin, end, isDendrogram);
        }
        
        // Vertical lines
        for (size_t i = 1; i <= mNumberOfNodes; i++) {
            PhyolgeticBranch *branch = mRoot->branchAt(i);
            
            if (branch->isInternal()) {
                Point begin(branch->getXChild(), branch->getYChildBegin());
                Point end(branch->getXChild(), branch->getYChildEnd());
                graphics.drawVerticalLine(begin, end);
            }
        }
        
        // Nodes
        for (size_t i = 1; i <= mNumberOfNodes; i++) {
            PhyolgeticBranch *branch = mRoot->branchAt(i);
            
            if (branch->isInternal()) {
                Point end(branch->getXEnd(), branch->getYEnd());
                graphics.drawPoint(end);
            }
        }
        
        
        // Labels
        for (size_t i = 1; i <= mNumberOfNodes; i++) {
            PhyolgeticBranch *branch = mRoot->branchAt(i);
            
            std::string s = branch->getLabel();
            if (s.length() > 0) {
                Point begin(branch->getXEnd(), branch->getYEnd());
                graphics.drawText(branch->getLabel(), begin);
            }
        }
        
        
        
        graphics.print();
    }
    
    
    void PhyolgeticTree::setRoot(PhyolgeticBranch *root)
    {
        mRoot = root;
    }
    
    void PhyolgeticTree::setMaxLengthToRoot()
    {
        mMaxLengthToRoot = mRoot->getMaxLengthToRoot();
    }
    
    int PhyolgeticTree::configure(double xScale)
    {
        mXScale = xScale;
        mRoot->computeLengthToRoot();
        setMaxLengthToRoot();
        mNumberOfNodes = mRoot->getSetNumberOfNodes();
        mRoot->setRowIndex(mNumberOfNodes);
        mRoot->setCoordinates(mMaxLengthToRoot, mXScale);
        mYScale = static_cast<double>(mNumberOfNodes);
        return 0;
    }
    
    void PhyolgeticTree::description()
    {
        configure();
        std::cout << "Tree: Nodes(" << mNumberOfNodes << ")" << std::endl;
        mRoot->description();
    }
    
    std::string
    PhyolgeticTree::toNewickString()
    {
        std::string s;
        mRoot->toNewickString(s);
        s.append(";");
        return s;
    }
    
#pragma mark - Generator
    
    PhyolgeticTree*
    TreeGenerator::generate(double maxLength, double length, double numberOfChildren)
    {
        mIndex = 1;
        mMaxLength = maxLength;
        mLength = length;
        mNumberOfChildren = numberOfChildren;
        PhyolgeticTree *t = new PhyolgeticTree;
        t->setRoot(sample());
        return t;
    }
    
    PhyolgeticBranch*
    TreeGenerator::sample(double lengthToRoot)
    {
        PhyolgeticBranch *branch = new PhyolgeticBranch;
        double length = mExponentialDistribution(mGenerator);
        //    int numberOfChildren = mPoissonDistribution(mGenerator);
        int numberOfChildren = 2;
        branch->setLength(length);
        branch->setLengthToRoot(length + lengthToRoot);
        if (branch->getLengthToRoot() < mMaxLength) {
            
            for (int i = 0; i < numberOfChildren; i++) {
                PhyolgeticBranch *childBranch = sample(branch->getLengthToRoot());
                branch->addChild(childBranch);
            }
        } else {
            std::string label = std::to_string(mIndex++);
            branch->setLabel(label);
        }
        return branch;
    }
#pragma mark - NewickParser
    
    NewickParser::NewickParser(std::string newickTree)
    : mIn(nullptr)
    {
        
#if defined(DEBUG_LOG)
        std::cout << "Parser: created." << std::endl;
#endif
        if (newickTree.length() > 0) {
            setNewickTree(newickTree);
        }
    }
    
    NewickParser::NewickParser(std::istream *is)
    : mIn(is)
    {
#if defined(DEBUG_LOG)
        std::cout << "Parser: created." << std::endl;
#endif
        //        mPosition = 0;
        //        initialize();
    }
    
    NewickParser::~NewickParser()
    {
#if defined(DEBUG_LOG)
        std::cout << "Parser: deleted." << std::endl;
#endif
    }
    
    void NewickParser::setNewickTree(std::string newickTree)
    {
        mNewickTree = newickTree;
        
        mPosition = 0;
        initialize();
    }
    
    PhyolgeticTree* NewickParser::parse()
    {
        PhyolgeticTree *t = tree();
        PhyolgeticBranch *root = descendants();
        if (root == nullptr) {
            delete t;
            return nullptr;
        }
        t->setRoot(root);
        return t;
    }
    
    
    
    
    /**
     *  Initializes the parser.
     *
     *  @return True if the first character is a left parenthesis, false
     * otherwise.
     */
    bool NewickParser::initialize()
    {
        mErrorCode = ErrorCode::okay;
        mInput.clear();
        reset();
        ignore();
        bool isLeftParenthesis = readFirstLeftParenthesis();
        
        return isLeftParenthesis;
    }
    
    /**
     *  Clears the current token in the buffer.
     *
     *  @return 0
     */
    int NewickParser::reset()
    {
        mBuffer.clear();
        return 0;
    }
    
    /**
     *  Retrieves a character from the source.
     *
     *  @return 0
     */
    int NewickParser::ignore()
    {
        if (mIn == nullptr) {
            mCharacter = mNewickTree.at(mPosition++);
        } else {
            mCharacter = mIn->get();
        }
        mInput += mCharacter;
        return 0;
    }
    
    /**
     *  Appends the current character to the buffer and reads the next
     * character.
     *
     *  @return 0
     */
    int NewickParser::read()
    {
        mBuffer += mCharacter;
        ignore();
        return 0;
    }
    
    /**
     *  Appends the current character to the buffer and reads the next
     * character by considering that single quote characters in a quoted label
     * are represented by two single quotes.
     *
     *  @return 0
     */
    int NewickParser::readForQuotedLabel()
    {
        mBuffer += mCharacter;
        checkForQuotedLabel();
        return 0;
    }
    
    /**
     *  Reads a next character in the quoted label.
     *
     *  Requirements: Single quote characters in a quoted label are represented
     * by two single quotes.
     *
     *  @return 0
     */
    int NewickParser::checkForQuotedLabel()
    {
        if (mIn == nullptr) {
            int firstCharacter = mNewickTree.at(mPosition);
            int secondCharacter = mNewickTree.at(mPosition + 1);
            // goshng: check the out-of-bound
            if (firstCharacter == '\'' && secondCharacter == '\'') {
                mCharacter = '\'';
                mPosition += 2;
                mInput += firstCharacter;
            } else {
                if (firstCharacter == '\'') {
                    mCharacter = 0;
                } else {
                    mCharacter = firstCharacter;
                }
                mPosition++;
            }
            
        } else {
            int firstCharacter = mIn->get();
            int secondCharacter = mIn->get();
            // goshng: check the out-of-bound
            if (firstCharacter == '\'' && secondCharacter == '\'') {
                mCharacter = '\'';
                //                mPosition += 2;
                mInput += firstCharacter;
                mInput += secondCharacter;
            } else {
                if (firstCharacter == '\'') {
                    mCharacter = 0;
                } else {
                    mCharacter = firstCharacter;
                }
                mIn->putback(secondCharacter);
                mInput += firstCharacter;
                //                mPosition++;
            }
        }
        return 0;
    }
    
    /**
     *  Extracts a next term in the source.
     *
     *  @return The operator type in the buffer.
     */
    int NewickParser::next()
    {
        mOperator = scan();
        return 0;
    }
    
    /**
     *  Reads the first left parenthesis.
     *
     *  @return True if the character is a left parenthesis, false otherwise.
     */
    bool NewickParser::readFirstLeftParenthesis()
    {
        mOperator = scan();
        if (mOperator == Operator::LEFT_PARENTHESIS) {
            return true;
        } else {
            mErrorCode = ErrorCode::noLeftParenthesis;
            return false;
        }
    }
    
    PhyolgeticTree*
    NewickParser::tree()
    {
        PhyolgeticTree *tree = new PhyolgeticTree;
        
        return tree;
    }
    
    PhyolgeticTree*
    NewickParser::nextTree()
    {
        mPosition = 0;
        bool isInitialized = initialize();
        
        if (mOperator == Operator::END_OF_FILE) {
            return nullptr;
        } else if (isInitialized == false) {
            putback();
            throw ParsingException();
            return nullptr;
        }
        
        PhyolgeticTree *t = tree();
        PhyolgeticBranch *root = subtree();
        //        PhyolgeticBranch *root = descendants();
        if (root == nullptr) {
            putback();
            delete t;
            throw ParsingException();
            return nullptr;
        }
        t->setRoot(root);
        //        assert(mOperator == Operator::SEMICOLON);
        if (mOperator != Operator::SEMICOLON) {
            delete t;
            return nullptr;
        }
        return t;
        
    }
    
    /**
     *
     *
     *  @return
     */
    PhyolgeticBranch*
    NewickParser::descendants()
    {
        PhyolgeticBranch *branch = new PhyolgeticBranch;
        
        PhyolgeticBranch *childBranch;
        childBranch = subtree();
        if (childBranch == nullptr) {
            delete branch;
            return nullptr;
        }
        
        branch->addChild(childBranch);
        while (mOperator == Operator::COMMA) {
            next();
            childBranch = subtree();
            if (childBranch == nullptr) {
                delete branch;
                return nullptr;
            }
            branch->addChild(childBranch);
        }
        return branch;
    }
    
    PhyolgeticBranch*
    NewickParser::label(PhyolgeticBranch *branch)
    {
        branch->setLabel(mBuffer);
        next();
        if (!(mOperator == Operator::COLON ||
              mOperator == Operator::COMMA ||
              mOperator == Operator::SEMICOLON ||
              mOperator == Operator::RIGHT_PARENTHESIS ||
              mOperator == Operator::LEFT_BRACKET)) {
            mErrorCode = ErrorCode::labelFollower;
            delete branch;
            return nullptr;
        }
        return branch;
    }
    
    PhyolgeticBranch*
    NewickParser::subtree()
    {
        PhyolgeticBranch *branch = nullptr;
        
        switch (mOperator) {
            case Operator::LEFT_PARENTHESIS:
                next();
                
                if (mOperator == Operator::RIGHT_PARENTHESIS) {
                    return nullptr;
                }
                
                branch = descendants();
                if (branch == nullptr) {
                    return nullptr;
                }
                
                if (mOperator != Operator::RIGHT_PARENTHESIS) {
                    mErrorCode = ErrorCode::noMatchingParentheses;
                    // tempo
                    int c = mIn->get();
                    mIn->putback(c);
                    
                    delete branch;
                    return nullptr;
                }
                next();
                if (mOperator == Operator::LABEL) {
                    // label of a branch.
                    //                    branch->setLabel(mBuffer);
                    //                    next();
                    branch = label(branch);
                    if (branch == nullptr) {
                        return nullptr;
                    }
                    
                }
                break;
            case Operator::LABEL:
                // label of a leaf.
                branch = new PhyolgeticBranch;
                branch = label(branch);
                if (branch == nullptr) {
                    return nullptr;
                }
                
                //                branch->setLabel(mBuffer);
                //                next();
                //                if (!(mOperator == Operator::COLON ||
                //                      mOperator == Operator::COMMA ||
                //                      mOperator == Operator::SEMICOLON ||
                //                      mOperator == Operator::RIGHT_PARENTHESIS ||
                //                      mOperator == Operator::LEFT_BRACKET)) {
                //                    mErrorCode = ErrorCode::labelFollower;
                //                    delete branch;
                //                    return nullptr;
                //                }
                
                break;
            default:
                mErrorCode = ErrorCode::subtree;
                return nullptr;
                break;
        }
        
        // Exception: Two labels in a row
        if (mOperator == Operator::LABEL) {
            assert(branch != nullptr);
            mErrorCode = ErrorCode::twoLabels;
            delete branch;
            return nullptr;
            
        }
        
        if (mOperator == Operator::COLON) {
            next();
            if (mOperator != Operator::NUMBER) {
                assert(branch != nullptr);
                mErrorCode = ErrorCode::noNumberAfterColon;
                delete branch;
                return nullptr;
            }
            // branch length.
            double length = std::stod(mBuffer);
            branch->setLength(length);
            next();
            
            // Requirements: Blanks or tabs may appear anywhere except within unquoted labels or
            // branch_lengths.
            if (!(mOperator == Operator::COMMA ||
                  mOperator == Operator::RIGHT_PARENTHESIS ||
                  mOperator == Operator::SEMICOLON)) {
                assert(branch != nullptr);
                mErrorCode = ErrorCode::number;
                delete branch;
                return nullptr;
            }
        } else {
            branch->setLengthForDendrogram();
        }
        return branch;
    }
    
    
    NewickParser::Operator
    NewickParser::scan()
    {
        int c;
    INIT:
        reset();
        
        if (mCharacter == EOF) {
            return Operator::END_OF_FILE;
        }
        
        if (mCharacter == ' ') {
            ignore();
        }
        
        if (mOperator == Operator::COLON) {
            while (!isLeadingNumber(mCharacter)) {
                if (isspace(mCharacter)) {
                    mErrorCode = ErrorCode::noNumberAfterColon;
                    return Operator::UNKNOWN;
                }
                ignore();
            }
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
                goto INIT;
                break;
            case ',':
                read();
                return Operator::COMMA;
            case '(':
                read();
                return Operator::LEFT_PARENTHESIS;
            case ')':
                if (mOperator == Operator::LEFT_PARENTHESIS) {
                    //                    return Operator::ERROR_NO_LABEL;
                    mErrorCode = ErrorCode::emptyDescendant;
                    return Operator::RIGHT_PARENTHESIS;
                }
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
                //                mCharacter = ' ';
                return Operator::COLON;
            case ';':
                // tempo
                //                c = mIn->get();
                //                mIn->putback(c);
                
                //                mCharacter = ' ';
                return Operator::SEMICOLON;
                //        case '\n':
                //            mCharacter = ' ';
                //            return Operator::END;
            case EOF:
                return Operator::END_OF_FILE;
                break;
            default:
                // Exception?
                throw ParsingException();
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
        ignore();
        // Trim the string.
        mBuffer = trim_copy(mBuffer);
        return Operator::LABEL;
    }
    
    // Requirements: Unquoted labels may not contain blanks, parentheses, square brackets,
    // single_quotes, colons, semicolons, or commas.
    bool
    NewickParser::isLabel(int ch)
    {
        if (isspace(ch) || iscntrl(ch)) {
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
    
    bool
    NewickParser::isLeadingNumber(int ch)
    {
        if (isspace(ch)) {
            return false;
        }
        if (isnumber(ch) || ch == '.' || ch == '+' || ch == '-') {
            return true;
        } else {
            return false;
        }
    }
    
    int
    NewickParser::skip()
    {
        if (mIn == nullptr) {
            assert(0);
        } else {
            mCharacter = mIn->get();
            //            mInput += mCharacter;
            while (mCharacter != '\n' && mCharacter != ';' && mCharacter != EOF) {
                mCharacter = mIn->get();
                mInput += mCharacter;
            }
            if (mCharacter == ';') {
                mOperator = Operator::SEMICOLON;
            } else if (mCharacter == EOF) {
                mOperator = Operator::END_OF_FILE;
            } else if (mCharacter == '\n') {
                mOperator = Operator::END_OF_LINE;
            }
        }
        return 0;
    }
    
    int
    NewickParser::putback()
    {
        if (mIn == nullptr) {
            assert(0);
        } else {
            //            // tempo
            //            int c = mIn->get();
            //            mIn->putback(c);
            mIn->putback(mCharacter);
            //            // tempo
            //            c = mIn->get();
            //            mIn->putback(c);
        }
        return 0;
    }
    
    std::string
    NewickParser::getInput() const
    {
        return mInput;
    }
    
    // Reference
    // http://stackoverflow.com/questions/3975313/translate-error-codes-to-string-to-display
    void NewickParser::printErrorMessage()
    {
        std::cerr << "Error: [code " << static_cast<int>(mErrorCode) << "] ";
        //        std::cerr << err2msg(mErrorCode) << std::endl;
        const std::map<ErrorCode, const char*>::const_iterator it = mErrorCodeTable.find(mErrorCode);
        if (it == mErrorCodeTable.end()) {
            std::cerr << "unknown";
        } else {
            std::cerr << it->second;
        }
        std::cerr << std::endl;
    }
    //
    //
    //        typedef std::map<int, const char*> error_code_tbl_t;
    //
    //        typedef error_code_tbl_t::value_type error_code_entry_t;
    //        const error_code_entry_t error_code_tbl_[] = {
    //            { ERR_OK              , "ERR_OK" },
    //            { ERR_NUMBER              , "Comma, right parenthesis, semi-colon must follow a number." },
    //            { ERR_RT_OUT_OF_MEMORY, "ERR_RT_OUT_OF_MEMORY" }
    //        };
    //        const error_code_tbl_t error_code_tbl( begin(error_code_tbl_)
    //                                              ,end  (error_code_tbl_) );
    //
    //        const char* err2msg(int code)
    //        {
    //            const error_code_tbl_t::const_iterator it = error_code_tbl.find(code);
    //            if(it == error_code_tbl.end())
    //                return "unknown";
    //            return it->second;
    //        }
#pragma mark - Graphics
    
    Point::Point(double x, double y)
    : mX(x), mY(y)
    {
        
    }
    
    Graphics::Graphics(size_t x, size_t y)
    : mX(x), mY(y)
    {
        mCoordinate.resize(y);
        for (std::vector<std::string>::iterator i = mCoordinate.begin(); i != mCoordinate.end(); i++) {
            i->assign(x, ' ');
        }
    }
    
    void Graphics::drawHorizontalLine(Point begin, Point end, bool isDendrogram)
    {
        
        size_t xBegin = static_cast<size_t>(std::round(begin.mX));
        size_t xEnd = static_cast<size_t>(std::round(end.mX));
        size_t yBegin = static_cast<size_t>(std::round(begin.mY));
        size_t yEnd = static_cast<size_t>(std::round(end.mY));
        std::string &line = mCoordinate.at(yBegin);
        for (size_t i = 0; i < line.size(); i++) {
            if (xBegin <= i && i <= xEnd) {
                if (isDendrogram) {
                    line.at(i) = '~';
                } else {
                    line.at(i) = '-';
                }
            }
        }
    }
    
    
    void Graphics::drawVerticalLine(Point begin, Point end)
    {
        size_t xBegin = static_cast<size_t>(std::round(begin.mX));
        size_t xEnd = static_cast<size_t>(std::round(end.mX));
        size_t yBegin = static_cast<size_t>(std::round(begin.mY));
        size_t yEnd = static_cast<size_t>(std::round(end.mY));
        
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
    
    void Graphics::drawPoint(Point end)
    {
        size_t xEnd = static_cast<size_t>(std::round(end.mX));
        size_t yEnd = static_cast<size_t>(std::round(end.mY));
        
        for (size_t i = 0; i < mCoordinate.size(); i++) {
            if (yEnd == i) {
                mCoordinate.at(i).at(xEnd) = '+';
            }
        }
    }
    
    void Graphics::drawText(std::string text, Point begin)
    {
        size_t xBegin = static_cast<size_t>(begin.mX);
        size_t yBegin = static_cast<size_t>(begin.mY);
        
        text.insert(0, 1, ' ');
        
        std::string &line = mCoordinate.at(yBegin);
        line.replace(xBegin, text.size(), text);
    }
    
    void Graphics::print()
    {
        for (std::vector<std::string>::iterator i = mCoordinate.begin(); i != mCoordinate.end(); i++) {
            std::cout << *i << std::endl;
        }
    }
    
}

















































