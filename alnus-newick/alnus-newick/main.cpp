//
//  main.cpp
//  alnus-newick
//
//  Created by Sang Chul Choi on 9/8/14.
//  Copyright (c) 2014 Sang Chul Choi. All rights reserved.
//

// Define BEBUG_LOG for logging more.
#define DEBUG_LOG
#undef DEBUG_LOG

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <random>
#include <stdexcept>
#include <map>
#include <sstream>

#include <cctype>
#include <cassert>
#include <cmath>

#pragma mark - classes

/**
 *  Alnus namespace contains most of the clasees:
 * PhyloeticBranch,
 * PhylogeticTree,
 * PhylogeneticTreeGenerator,
 * NewickParser,
 * Point,
 * Graphics.
 */
namespace Alnus {
    
    /**
     *  NewickParserContant stores the constant values.
     */
    class NewickParserContant {
    public:
        static const double ThresholdLength;
        static const double MeanLength;
        static const double MeanNumberOfChildren;
        static const int EndOfLine;
        static const int Space;
        static const int LeftParenthesis;
        static const int RightParenthesis;
        static const int QuotationMark;
        static const int LeftBracket;
        static const int RightBracket;
        static const int Comma;
        static const int Colon;
        static const int Semicolon;
        static const int Plus;
        static const int Minus;
        static const int Tab;
        static const int LineFeed;
        static const int Period;
        static const int Underscore;
        static const int Tilde;
        static const int Slash;
        static const int Pipe;
        static const int Backslash;
        static const int E;
        static const int e;
        static const size_t GraphicsXSize;
        static const size_t GraphicsYSize;
        static const double LengthForDendrogram;
        static const double ScalebarBegin;
        static const double ScalebarEnd;
        static const double ScalebarSize;
    };
    
    const double NewickParserContant::ThresholdLength      = 1.0;
    const double NewickParserContant::MeanLength           = 1.0;
    const double NewickParserContant::MeanNumberOfChildren = 2;
    const int NewickParserContant::EndOfLine               = '\n';
    const int NewickParserContant::Space                   = ' ';
    const int NewickParserContant::LeftParenthesis         = '(';
    const int NewickParserContant::RightParenthesis        = ')';
    const int NewickParserContant::QuotationMark           = '\'';
    const int NewickParserContant::LeftBracket             = '[';
    const int NewickParserContant::RightBracket            = ']';
    const int NewickParserContant::Comma                   = ',';
    const int NewickParserContant::Colon                   = ':';
    const int NewickParserContant::Semicolon               = ';';
    const int NewickParserContant::Plus                    = '+';
    const int NewickParserContant::Minus                   = '-';
    const int NewickParserContant::Tab                     = '\t';
    const int NewickParserContant::LineFeed                = '\r';
    const int NewickParserContant::Period                  = '.';
    const int NewickParserContant::Underscore              = '_';
    const int NewickParserContant::Tilde                   = '~';
    const int NewickParserContant::Slash                   = '/';
    const int NewickParserContant::Pipe                    = '|';
    const int NewickParserContant::Backslash               = '\\';
    const int NewickParserContant::E                       = 'E';
    const int NewickParserContant::e                       = 'e';
    const size_t NewickParserContant::GraphicsXSize        = 50;
    const size_t NewickParserContant::GraphicsYSize        = 50;
    const double NewickParserContant::LengthForDendrogram  = 1.0;
    const double NewickParserContant::ScalebarBegin        = 0.2;
    const double NewickParserContant::ScalebarEnd          = 0.4;
    const double NewickParserContant::ScalebarSize         = NewickParserContant::ScalebarEnd - NewickParserContant::ScalebarBegin;
    
    /**
     *  NewickParserRun is the main proxy.
     */
    class NewickParserRun {
    public:
        static void printHelp();
        static void printTestNewickTree();
        static void mainForNewickFileWithOutNewick(std::istream &inFile,
                                                   std::ostream &outFile,
                                                   bool useOutFile);
    };
    
    /**
     *  catches parsing exceptions.
     */
    class ParsingException : public std::runtime_error
    {
    public:
        ParsingException();
    };
    
    /**
     *  PhylogeticBranch represents a branch in a phylogetic tree.
     * 
     *  Two attributes of PhylogeticBranch class are the length and the label
     * of a branch. The length of a branch is a number. A positive number or
     * zero is a usually. A negative number is possible although it raresy
     * makes sense. The label of a branch names the tip of the branch. The 
     * label can be empty.
     */
    class PhylogeticBranch
    {
    public:
        // Constructor and destructor
        PhylogeticBranch(double length = 0, std::string label = "");
        ~PhylogeticBranch();
        
        // Accessors
        static size_t getReferenceCount();
        void setLength(const double length);
        void setLabel(const std::string label);
        double getLengthToRoot() const;
        void setLengthToRoot(const double lengthToRoot);
        unsigned int getNumberOfNodes() const;
        double getXBegin() const;
        double getYBegin() const;
        double getXEnd() const;
        double getYEnd() const;
        bool isInternal() const;
        bool isExternal() const;
        double getXChild() const;
        double getYChildBegin() const;
        double getYChildEnd() const;
        std::string getLabel() const;
        bool isDendrogram() const;
        void setIsDendrogram(const bool isDendrogram);
        int enumerateBranch(std::vector<PhylogeticBranch*> &list);
        std::string getStringOfLength() const;
        void setStringOfLength(std::string s);
        void setQuotedLabel(const bool quotedLabel);
        bool isQutedLabel() const;
        
        // Modifiers
        void addChild(PhylogeticBranch *child);
        void computeLengthToRoot(const double lengthToRootOfParent = 0);
        double getMaxLengthToRoot();
        unsigned int getSetNumberOfNodes();
        void setRowIndex(const unsigned int baseIndex);
        void setCoordinates(double x_max,
                            double x_scale = NewickParserContant::GraphicsXSize);
        void setLengthForDendrogram();
        
        // Output
        void description();
        std::string makeQuotedLabel() const;
        void toNewickString(std::string& s);
        
        PhylogeticBranch* branchAt(size_t i);
        
    private:
        /**
         *  Represents the number of branch objects created by the 
         * PhylogeneticBranch class.
         */
        static size_t mReferenceCount;
        
        /**
         *  Represents the length of a branch.
         */
        double mLength;
        
        /**
         *  Represents the length of a branch just read from the
         * Newick string. This is only used for checking the parsing
         * procedure.
         */
        std::string mStringOfLength;
        
        /**
         *  Represents the name of the tailing node in a branch.
         */
        std::string mLabel;
        
        /**
         *  Represents the boolean for whether the label is quoted.
         */
        bool mQuotedLabel;
        
        /**
         *  Represents a list of children branch objects.
         */
        std::vector<PhylogeticBranch*> mChildren;
        
        /**
         *  Represents the length to the root node from the tailing node of a
         * branch. After a tree is built, the length to the root can be 
         * updated. Therefore, it is optional when creating a branch object.
         * When creating a branch object, the value is set to -1.
         */
        double mLengthToRoot;
        
        /**
         *  Represents the number of nodes below a branch. It is optional
         * when creating a branch object. After a tree is built, the number
         * of nodes below a branch is counted. When creating a branch object,
         * the number of nodes below the branch is set to 0. After counting
         * nodes, the leaf branch's number of nodes is 1.
         */
        unsigned int mNumberOfNodes;
        
        /**
         *  Represents the row index at which a branch is drawn in a text-baesd
         * output.
         */
        unsigned int mRowIndex;
        
        /**
         *  Represents the coordinates of two points in a text-based screen.
         * The branch draws itself as a horizontal line that begins from the
         * left and ends towards the right.
         */
        double mXBegin;
        double mYBegin;
        double mXEnd;
        double mYEnd;
        
        /**
         *  Represents the number of characters in a horizontal line.
         */
        double mXScale;
        
        /**
         *  Represents the boolean value for whether a branch is part of a
         * dendrogram. If the branch length of a branch is unspecified, a 
         * default value is given and this data member is set to true.
         */
        bool mIsDendrogram;
        
        /**
         *  Represents the default length of a branch if the length is 
         * unspecified.
         */
        static const double mLengthForDendrogram;
    };
    
    size_t PhylogeticBranch::mReferenceCount = 0;
    const double PhylogeticBranch::mLengthForDendrogram = NewickParserContant::LengthForDendrogram;
    
    /**
     *  PhylogeneticTree replesents a phylogenetic tree.
     */
    class PhylogeneticTree
    {
    public:
        // Constructor and destructor
        PhylogeneticTree();
        ~PhylogeneticTree();
        
        // Accessors
        void setRoot(PhylogeticBranch *root);
        void setMaxLengthToRoot();
        int configure(double xScale = NewickParserContant::GraphicsXSize);
        
        // Output
        void description();
        void draw();
        std::string toNewickString();
        
    private:
        /**
         *  Represents the root of a phylogenetic tree.
         */
        PhylogeticBranch *mRoot;
        
        /**
         *  Represents the maximum length of a tree.
         */
        double mMaxLengthToRoot;
        
        /**
         *  Represents the number of nodes in a tree.
         */
        unsigned int mNumberOfNodes;
        
        /**
         *  Represents the size of a canvas.
         */
        double mXScale;
        double mYScale;
        
        /**
         *  Represents the coordinates of two points in a text-based screen.
         * A scale bar connects the two points.
         */
        double mXBegin;
        double mYBegin;
        double mXEnd;
        double mYEnd;
        double mScaleBarSize;
        
        /**
         *  Represents the list of branch object pointers.
         */
        std::vector<PhylogeticBranch*> mListOfBranches;
    };
    
    /**
     *  PhylogeneticTreeGenerator randomly generates a tree.
     */
    class PhylogeneticTreeGenerator
    {
    public:
        static PhylogeneticTree* generate(double maxLength = NewickParserContant::ThresholdLength,
                                          double length = NewickParserContant::MeanLength,
                                          double numberOfChildren = NewickParserContant::MeanNumberOfChildren);
    private:
        static PhylogeticBranch* sample(std::poisson_distribution<int> &poissonDistribution,
                                        std::exponential_distribution<double> &exponentialDistribution,
                                        double lengthToRoot = 0,
                                        bool isRoot = false);
        
        /**
         *  Represents the threshold of the length from external nodes to the
         * root of a tree. A tree grows till the length to the root reaches 
         * this threshold.
         */
        static double mMaxLength;
        
        /**
         *  Represents the average rate of a branching event distributed as an
         * exponential.
         */
        static double mLength;
        
        /**
         *  Represents the mean number of children followed by a Poisson
         * distribution.
         */
        static double mNumberOfChildren;
        
        /**
         *  Represents the index for labeling a branch.
         */
        static unsigned int mIndex;
        
        /**
         *  Represents the pseudo-random number generator.
         */
        static std::default_random_engine mGenerator;
        
    };
    
    double PhylogeneticTreeGenerator::mMaxLength = NewickParserContant::ThresholdLength;
    double PhylogeneticTreeGenerator::mLength = NewickParserContant::MeanLength;
    double PhylogeneticTreeGenerator::mNumberOfChildren = NewickParserContant::MeanNumberOfChildren;
    unsigned int PhylogeneticTreeGenerator::mIndex = 1;
    std::default_random_engine PhylogeneticTreeGenerator::mGenerator;
    
    /**
     *  NewickParser parses a stream for generating phylogenetic trees.
     */
    class NewickParser
    {
    public:
        // Constructor and destructor
        NewickParser(std::istream *in);
        ~NewickParser();
        
        PhylogeneticTree* nextTree();
        int skip();
        
        // Accessors
        std::string getInput() const;
        void setNewickTree(std::string newickTree);
        
        // Output
        void printErrorMessage();
        
        /**
         *  Operators used in parsing of a Newick tree string.
         */
        enum class Operator {
            LEFT_PARENTHESIS, 
            RIGHT_PARENTHESIS, 
            QUOTATION_MARK,
            LEFT_BRACKET, 
            RIGHT_BRACKET,
            NUMBER, 
            END_OF_FILE,
            END_OF_LINE,
            LABEL, 
            QUOTED_LABEL,
            COMMA, 
            COLON, 
            SEMICOLON,
            ERROR_NO_LABEL,
            ERROR_NO_NUMBER,
            UNKNOWN
        };
        
        /**
         *  Error codes during parsing of a Newick tree string.
         */
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
        // Parser's helper methods
        bool initialize();
        int reset();
        int ignore();
        int read();
        int nextForQuotedLabel();
        int checkForQuotedLabel();
        int next();
        bool nextFirstLeftParenthesis();
        
        // Production rules
        PhylogeneticTree* tree();
        PhylogeticBranch* descendants();
        PhylogeticBranch* label(PhylogeticBranch *branch);
        PhylogeticBranch* subtree();
        Operator scan();
        bool isLabel(int ch);
        void skipSpaces();
        bool isLeadingNumber(int ch);
        int putback();
        
    private:
        /**
         *  Represents the input stream.
         */
        std::istream *mIn;
        
        /**
         *  Represents the operator type in LL(1) parser.
         */
        Operator mOperator;
        
        /**
         *  Represents the parsed item on the current operator.
         */
        std::string mBuffer;
        
        /**
         *  Represents the current character from the input stream.
         */
        int mCharacter;
        
        /**
         *  Represents the total string read for parsing a single Newick tree.
         */
        std::string mInput;
        
        /**
         *  Represents the error code while parsing the input stream.
         */
        ErrorCode mErrorCode;
    };
    
    const std::map<NewickParser::ErrorCode, const char*>::value_type NewickParser::mErrorTable[] = {
        { NewickParser::ErrorCode::okay, "No error." },
        { NewickParser::ErrorCode::number, "Comma, right parenthesis, or semi-colon must follow a number." },
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
    
    /**
     *  Point represents a point in the text-based screen.
     */
    class Point
    {
    public:
        Point(double x = 0, double y = 0);
        double mX;
        double mY;
    };
    
    /**
     *  Graphics represents the text-based screen for drawing a phylogenetic
     * tree.
     */
    class Graphics
    {
    public:
        // Constructor
        Graphics(size_t x = NewickParserContant::GraphicsXSize,
                 size_t y = NewickParserContant::GraphicsYSize);
        
        // Modifiers
        void drawHorizontalLine(Point begin, Point end, bool isDendrogram = false);
        void drawVerticalLine(Point begin, Point end);
        void drawPoint(Point end, char ch = NewickParserContant::Plus);
        void drawText(std::string text, Point position);
        
        // Output
        void print();
    private:
        /**
         *  The size of a canvas.
         */
        size_t mX;
        size_t mY;
        
        /**
         *  The canvas.
         */
        std::vector<std::string> mCoordinate;
    };
    
    /**
     *  Helps a branch object free the memory of its children branch objects.
     */
    template<class T>
    class DeleteVector
    {
    public:
        bool operator()(T x) const
        {
            delete x;
            return true;
        }
    };
    
    /**
     *  Helps a branch object print the state of its children branch objects.
     */
    template<class T>
    class CallDescriptionVector
    {
    public:
        bool operator()(T x) const
        {
            x->description();
            return true;
        }
    };
    
    /**
     *  reverse_range is for iterating a vector in the reverse
     * order. See
     * http://www.codesynthesis.com/~boris/blog/2012/05/16/cxx11-range-based-for-loop/
     */
    template <typename T>
    struct reverse_range
    {
    private:
        T& x_;
        
    public:
        reverse_range (T& x): x_ (x) {}
        
        auto begin () const -> decltype (this->x_.rbegin ())
        {
            return x_.rbegin ();
        }
        
        auto end () const -> decltype (this->x_.rend ())
        {
            return x_.rend ();
        }
    };
    
    template <typename T>
    reverse_range<T> reverse_iterate (T& x)
    {
        return reverse_range<T> (x);
    }
    
    /**
     *  Trims a string. We use the following functions instead of those of
     * Boost. See
     * http://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring
     */
    inline std::string trim_right_copy(const std::string& s,
                                       const std::string& delimiters = " \f\n\r\t\v" )
    {
        return s.substr( 0, s.find_last_not_of( delimiters ) + 1 );
    }
    
    inline std::string trim_left_copy(const std::string& s,
                                      const std::string& delimiters = " \f\n\r\t\v" )
    {
        return s.substr( s.find_first_not_of( delimiters ) );
    }
    
    inline std::string trim_copy(const std::string& s,
                                 const std::string& delimiters = " \f\n\r\t\v" )
    {
        return trim_left_copy( trim_right_copy( s, delimiters ), delimiters );
    }
}

#pragma mark - main


/**
 *  The main function.
 *
 *  @return 0 for success, 1 for failure.
 */
int main(int argc, const char * argv[]) {
    
    if (argc == 1) {
        std::cout << "alnus-newick version 1.0" << std::endl;
        std::cout << "For help, type alnus-newick -h" << std::endl;
        std::cout << "Please, type a string in Newick Tree format and ENTER key." << std::endl;
        std::cout << "Push Control and D at the same time to quit." << std::endl;
        Alnus::NewickParserRun::mainForNewickFileWithOutNewick(std::cin,
                                                               std::cout,
                                                               false);
    } else if (argc == 2 && !strcmp(argv[1], "-h")) {
        Alnus::NewickParserRun::printHelp();
    } else if (argc == 2 && !strcmp(argv[1], "-t")) {
        Alnus::NewickParserRun::printTestNewickTree();
    } else if (argc == 3 && !strcmp(argv[1], "-g")) {
        int numberOfTrees = std::stoi(argv[2]);
        for (int i = 0; i < numberOfTrees; i++) {
            Alnus::PhylogeneticTree *t = Alnus::PhylogeneticTreeGenerator::generate();
            std::cout << t->toNewickString() << std::endl;
            delete t;
        }
    } else if (argc == 9 &&
               !strcmp(argv[1], "-g") &&
               !strcmp(argv[3], "-p") &&
               !strcmp(argv[5], "-e") &&
               !strcmp(argv[7], "-l")
               ) {
        int numberOfTrees = std::stoi(argv[2]);
        int meanNumberOfChildren = std::stoi(argv[4]);
        double meanWaitingTime = std::stod(argv[6]);
        double thresholdTimeToRood = std::stod(argv[8]);
        for (int i = 0; i < numberOfTrees; i++) {
            Alnus::PhylogeneticTree *t =
            Alnus::PhylogeneticTreeGenerator::generate(thresholdTimeToRood,
                                                       meanWaitingTime,
                                                       meanNumberOfChildren);
            std::cout << t->toNewickString() << std::endl;
            delete t;
        }
    } else if (argc == 3 && !strcmp(argv[1], "-o")) {
        // command -o out.tre
        std::ofstream outFile(argv[2]);
        Alnus::NewickParserRun::mainForNewickFileWithOutNewick(std::cin,
                                                               outFile,
                                                               true);
        
        outFile.close();
    } else if (argc == 4 && !strcmp(argv[1], "-o")) {
        std::ifstream inFile;
        std::ios_base::iostate exceptionMask = inFile.exceptions() | std::ios::failbit;
        inFile.exceptions(exceptionMask);
        
        bool canOpenAllFiles = true;
        try {
            inFile.open(argv[3]);
        } catch (std::ios_base::failure& e) {
            std::cerr << "Failed to open file: " << argv[3] << std::endl;
            canOpenAllFiles = false;
        }

        if (canOpenAllFiles == true) {
            std::ofstream outFile(argv[2]);
            Alnus::NewickParserRun::mainForNewickFileWithOutNewick(inFile,
                                                                   outFile,
                                                                   true);
            outFile.close();
            inFile.close();
        }
        
    } else {
        std::vector<std::string> arguments(argv + 1, argv + argc);
        
        bool canOpenAllFiles = true;
        for (auto &s : arguments) {
            std::ifstream inFile;
            std::ios_base::iostate exceptionMask = inFile.exceptions() | std::ios::failbit;
            inFile.exceptions(exceptionMask);
            
            try {
                inFile.open(s);
            } catch (std::ios_base::failure& e) {
                canOpenAllFiles = false;
                std::cerr << "Failed to open file: " << s << std::endl;
                break;
            }
            
            inFile.close();
        }
        
        if (canOpenAllFiles == true) {
            for (auto &s : arguments) {
                std::ifstream inFile;
                std::ios_base::iostate exceptionMask = inFile.exceptions() | std::ios::failbit;
                inFile.exceptions(exceptionMask);
                
                try {
                    inFile.open(s);
                } catch (std::ios_base::failure& e) {
                    std::cerr << "Failed to open file: " << s << std::endl;
                    break;
                }
                
                Alnus::NewickParserRun::mainForNewickFileWithOutNewick(inFile,
                                                                       std::cout,
                                                                       false);
                inFile.close();
            }
        }
    }
    
    return 0;
}


namespace Alnus
{
    
#pragma mark - Run
    void NewickParserRun::printHelp()
    {
        std::cout <<
        "alnus-newick version 1.0\n"
        "Author: Sang Chul Choi"
        "\n"
        "alnus-newick parses an input stream for a Newick tree string.\n"
        "\n"
        "Usage: alnus-newick FILES\n"
        "       alnus-newick -o FILE\n"
        "       alnus-newick -o FILE1 FILE2\n"
        "       alnus-newick -t\n"
        "       alnus-newick -g 10\n"
        "       alnus-newick -g 10 -n 2 -e 1.0 -l 1.0\n"
        "\n"
        "alnus-newick parses an input steam either from the standard input or files\n"
        "for Newick tree strings. It prints out the parsed Newick trees. The order\n"
        "of command options is strict. A list of examples for executing the cammand\n"
        "follows.\n"
        "\n"
        "Example: alnus-newick -t > file.tre\n"
        "  This prints out Newick tree strings that haved been tested.\n"
        "\n"
        "Example: alnus-newick file.tre\n"
        "  It parses the content of file file.tre for Newick tree strings.\n"
        "\n"
        "Example: alnus-newick -g 10\n"
        "  It generates 10 trees.\n"
        "\n"
        "Example: alnus-newick -g 5 -p 2 -e 1.0 -l 2.0 > random.tre\n"
        "  It generates 5 trees with mean number of children [Poisson] (2),\n"
        "  rate of events [Exponential] (1.0), and threshold time to the root (2.0).\n"
        "\n"
        "Example: alnus-newick -o random.out random.tre\n"
        "  It parses the file random.tre, and prints the Newick tree string to\n"
        "  random.out. The two files should be of the same.\n"
        "\n"
        "\n";
    }
    
    void NewickParserRun::printTestNewickTree()
    {
        std::cout <<
        "(A:  1e-1);\n"
        "(A);\n"
        "(A)R;\n"
        "(A)R:0.5;\n"
        "(A,B);\n"
        "(A,B,C);\n"
        "(A,B,C,D);\n"
        "(A,(B,C));\n"
        "(A,(B,C,D));\n"
        "(A:0.2,B:1.2):3;\n"
        "(A:3.2,(B:2.7,C:1.7))R:0.2;\n"
        "(A:.2,B:.8):.3;\n"
        "( 'a A a' ) ;\n"
        "('  ABC   ');\n"
        "(' A B ')' R o o t ';\n"
        "( 'A  B', 'A  C' );\n"
        "('  A''B  ', ' ''A''B ''  ');\n"
        "(A_B);\n"
        "(A_1,B_2,C_3);\n"
        "(  A  );\n"
        "(  A     )   ;\n"
        "(  A  )   R;\n"
        "A;\n"
        "A:0.1;\n"
        "(A B);\n"
        "(A)R o o t\n"
        "(A):0.1 2\n"
        "(  );\n"
        "(A)R T;\n"
        "( A ( );\n"
        "(A ,);\n"
        "( A: ) ;\n"
        "(A ‘ );\n"
        "( A ; );\n"
        "(A[ ]) ;\n"
        "(   A  \n"
        "   )   ;  \n"
        "\n"
        "(  A  ,  B  ,  \n"
        " C  ,  D  \n"
        " )\n"
        ";\n"
        "\n"
        "( A [qqcoantwotocuotuo] , [xotohaono] B ,  \n"
        " [otounont] C , [otnnhoe] [onthuoa]  \n"
        " D  [ohtose]  \n"
        " )  ;  \n"
        "(A:1e5);\n"
        "(A: 1e-1);\n"
        "(((One:0.2,Two:0.3):0.3,(Three:0.5,Four:0.3):0.2):0.3,Five:0.7):0.0;\n"
        "\n";
    }
    
    void
    NewickParserRun::mainForNewickFileWithOutNewick(std::istream &inFile,
                                                    std::ostream &outFile,
                                                    bool useOutFile)
    {
        NewickParser parser(&inFile);
        PhylogeneticTree *t = nullptr;
        
        while (!inFile.eof()) {
            
            try {
                t = parser.nextTree();
            } catch (ParsingException &unknownExeption) {
                parser.skip();
                std::cout << "Input: [" << parser.getInput() << "]" << std::endl;
                parser.printErrorMessage();
                std::cout << std::endl;
                assert(PhylogeticBranch::getReferenceCount() == 0);
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
                assert(PhylogeticBranch::getReferenceCount() == 0);
                continue;
            }
            
            std::cout << "Input: [";
            std::cout << parser.getInput();
            std::cout << "]";
            std::cout << std::endl;
            if (useOutFile == true) {
                outFile << t->toNewickString() << std::endl;
            }
#if defined(DEBUG_LOG)
            t->description();
#endif
            t->draw();
            delete t;
            std::cout << std::endl;
            assert(PhylogeticBranch::getReferenceCount() == 0);
        }
        
        
        if (inFile.eof()) {
#if defined(DEBUG_LOG)
            std::cout << "Reached the end of file!" << std::endl;
#endif
        }
    }

#pragma mark - Exception

    ParsingException::ParsingException()
    : std::runtime_error("Unknown parsing error")
    {
        
    }

#pragma mark - Branch

    /**
     *  Constructs a branch with a length and a name.
     *
     *  @param length The length of a branch.
     *  @param label  The name of a branch.
     */
    PhylogeticBranch::PhylogeticBranch(double length,
                                       std::string label)
    : mLength(length), mLabel(label), mIsDendrogram(false), mNumberOfNodes(0),
    mLengthToRoot(-1.0), mQuotedLabel(false)
    {
#if defined(DEBUG_LOG)
        std::cout << "Branch: created " << std::endl;
#endif
        mReferenceCount++;
    }
    
    /**
     *  Destructs a branch object.
     *
     *  It also frees the memory for all of the children branch objects.
     */
    PhylogeticBranch::~PhylogeticBranch()
    {
        for_each(mChildren.begin(), mChildren.end(),
                 DeleteVector<PhylogeticBranch*>());
        mChildren.clear();
        mReferenceCount--;
#if defined(DEBUG_LOG)
        std::cout << "Branch: deleted [" << mLabel << "]:" << mLength << std::endl;
#endif
    }
    
    /**
     *  Returns the number of branch objects created by the PhylogeticBranch
     * class.
     *
     *  @return The number of branch objects created by the PhylogeticBranch
     * class.
     */
    size_t PhylogeticBranch::getReferenceCount()
    {
        return mReferenceCount;
    }
    
    /**
     *  Sets the length of a branch.
     *
     *  @param length The length of a branch.
     */
    void PhylogeticBranch::setLength(const double length)
    {
        mLength = length;
    }
    
    /**
     *  Sets the name of the tailing node in a branch.
     *
     *  @param label The name of the tailing node in a branch.
     */
    void PhylogeticBranch::setLabel(const std::string label)
    {
        mLabel = label;
    }
    
    /**
     *  Gets the length to the root node from the tailing node of a branch.
     *
     *  @return The length to the root node from the tailing node of a branch.
     */
    double PhylogeticBranch::getLengthToRoot() const
    {
        return mLengthToRoot;
    }
    
    /**
     *  Sets the length to the root node from the tailing node of a branch.
     *
     *  @param lengthToRoot The length to the root node from the tailing node
     * of a branch.
     */
    void PhylogeticBranch::setLengthToRoot(const double lengthToRoot)
    {
        mLengthToRoot = lengthToRoot;
    }
    
    /**
     *  Returns the number of nodes below a branch.
     */
    unsigned int
    PhylogeticBranch::getNumberOfNodes() const
    {
        return mNumberOfNodes;
    }
    
    /**
     *  Returns the X-coordinate of the leading point of a branch.
     *
     *  @return The X-coordinate of the leading point of a branch.
     */
    double PhylogeticBranch::getXBegin() const
    {
        return mXBegin;
    }
    
    /**
     *  Returns the Y-coordinate of the leading point of a branch.
     *
     *  @return The Y-coordinate of the leading point of a branch.
     */
    double PhylogeticBranch::getYBegin() const
    {
        return mYBegin;
    }
    
    /**
     *  Returns the X-coordinate of the tailing point of a branch.
     *
     *  @return The X-coordinate of the tailing point of a branch.
     */
    double PhylogeticBranch::getXEnd() const
    {
        return mXEnd;
    }
    
    /**
     *  Returns the Y-coordinate of the tailing point of a branch.
     *
     *  @return The Y-coordinate of the tailing point of a branch.
     */
    double PhylogeticBranch::getYEnd() const
    {
        return mYEnd;
    }
    
    /**
     *  Returns true if a branch is internal.
     *
     *  @return True if a branch is internal, false otherwise.
     */
    bool PhylogeticBranch::isInternal() const
    {
        if (mChildren.size() == 0) {
            return false;
        } else {
            return true;
        }
    }
    
    /**
     *  Returns true if a branch is external.
     *
     *  @return True if a branch is external, false otherwise.
     */
    bool PhylogeticBranch::isExternal() const
    {
        return !isInternal();
    }

    /**
     *  Returns the X-coordinate of the leading point of a child branch.
     *
     *  @return The X-coordinate of the leading point of a child branch.
     */
    double PhylogeticBranch::getXChild() const
    {
        return getXEnd();
    }

    /**
     *  Returns the Y-coordinate of the leading point of a child branch.
     *
     *  @return The Y-coordinate of the leading point of a child branch.
     */
    double PhylogeticBranch::getYChildBegin() const
    {
        return mChildren.front()->getYBegin();
    }
    
    /**
     *  Returns the Y-coordinate of the tailing point of a child branch.
     *
     *  @return The Y-coordinate of the tailing point of a child branch.
     */
    double PhylogeticBranch::getYChildEnd() const
    {
        return mChildren.back()->getYBegin();
    }
    
    /**
     *  Returns the label of a branch.
     *
     *  @return The label of a branch.
     */
    std::string PhylogeticBranch::getLabel() const
    {
        return mLabel;
    }
    
    /**
     *  Returns true if the branch is one in a dendrogram. The branch length
     * is invalid.
     *
     *  @return True if the branch is one in a dendrogram. The branch length
     * is invalid, false otherwise.
     */
    bool PhylogeticBranch::isDendrogram() const
    {
        return mIsDendrogram;
    }
    
    /**
     *  Sets the boolean value for whether a branch is for a dendrogram.
     *
     *  @param isDendrogram The boolean value for whether a branch is for a 
     * dendrogram.
     */
    void PhylogeticBranch::setIsDendrogram(const bool isDendrogram)
    {
        mIsDendrogram = isDendrogram;
    }
    
    int PhylogeticBranch::enumerateBranch(std::vector<PhylogeticBranch*> &list)
    {
        list.push_back(this);
        for (auto &i : mChildren) {
            i->enumerateBranch(list);
        }
        return 0;
    }

    /**
     *  Gets the string of the length.
     *
     *  @return The string of the length.
     */
    std::string PhylogeticBranch::getStringOfLength() const
    {
        return mStringOfLength;
    }
    
    /**
     *  Sets the string of the length.
     *
     *  @param s The string of the length.
     */
    void PhylogeticBranch::setStringOfLength(std::string s)
    {
        mStringOfLength = s;
    }
    
    void PhylogeticBranch::setQuotedLabel(const bool quotedLabel)
    {
        mQuotedLabel = quotedLabel;
    }
    
    bool PhylogeticBranch::isQutedLabel() const
    {
        return mQuotedLabel;
    }
    
    /**
     *  Appends a branch to the list of children branch objects.
     *
     *  @param child The branch object.
     */
    void PhylogeticBranch::addChild(PhylogeticBranch *child)
    {
        mChildren.push_back(child);
    }
    
    /**
     *  Sets the length to the root node.
     *
     *  @param lengthToRootOfParent The length to the root node from the parent
     * branch.
     *
     *
     */
    void PhylogeticBranch::computeLengthToRoot(const double lengthToRootOfParent)
    {
        setLengthToRoot(mLength + lengthToRootOfParent);
        for (auto &i : mChildren) {
            i->computeLengthToRoot(mLengthToRoot);
        }
    }
    
    /**
     *  Returns the maximum length to the root.
     */
    double PhylogeticBranch::getMaxLengthToRoot()
    {
        double maxLength = getLengthToRoot();
        for (auto &i : mChildren) {
            double lengthToRoot = i->getMaxLengthToRoot();
            if (maxLength < lengthToRoot) {
                maxLength = lengthToRoot;
            }
        }
        return maxLength;
    }
    
    /**
     *  Returns the number of nodes below a branch.
     */
    unsigned int
    PhylogeticBranch::getSetNumberOfNodes()
    {
        mNumberOfNodes = 0;
        for (auto &i : mChildren) {
            mNumberOfNodes += i->getSetNumberOfNodes();
        }
        return mNumberOfNodes + 1;
    }
    
    /**
     *  Configures the row index of a branch and those of its children branch
     * objects.
     *
     *  @param baseIndex The base index.
     *
     *  This sets mRowIndex for drawing the tree with a branch object.
     */
    void
    PhylogeticBranch::setRowIndex(const unsigned int baseIndex)
    {
        if (mChildren.empty()) {
            mRowIndex = baseIndex;
            return;
        }
        
        // Cumulative numbers of nodes in the reverse order
        std::vector<unsigned int> numbersOfNodesInRightChildren;
        unsigned int numberOfNodes = 0;
        numbersOfNodesInRightChildren.push_back(numberOfNodes);
        
        for (auto &i : reverse_iterate(mChildren)) {
            numberOfNodes += i->getNumberOfNodes() + 1;
            numbersOfNodesInRightChildren.push_back(numberOfNodes);
        }
        
        // Recursive calls
        mRowIndex = baseIndex - numbersOfNodesInRightChildren.at(mChildren.size() - 1);
        PhylogeticBranch *branch = mChildren.at(0);
        branch->setRowIndex(mRowIndex - 1);
        
        for (size_t i = 1; i < mChildren.size(); i++) {
            branch = mChildren.at(i);
            unsigned int childBaseIndex = baseIndex - numbersOfNodesInRightChildren.at(mChildren.size() - 1 - i);
            branch->setRowIndex(childBaseIndex);
        }    
    }
    
    /**
     *  Sets the coordinates of a branch object.
     *
     *  @param x_max   The largest length to the root in a tree.
     *  @param x_scale The number of characters along the X-axis.
     *
     *  You can change the width of a tree with a larger x_scale. Use a tree
     * to find the longest branches from a leaf node to the root in the tree.
     * Put the length in x_max.
     */
    void PhylogeticBranch::setCoordinates(double x_max, double x_scale)
    {
        mXScale = x_scale;
        mXEnd = mLengthToRoot / x_max * x_scale;
        mYEnd = mRowIndex;
        mXBegin = mXEnd - (mLength / x_max * x_scale);
        mYBegin = mYEnd;
        
        for (auto &i : mChildren) {
            i->setCoordinates(x_max, x_scale);
        }   
    }
    
    /**
     *  Makes a branch for a dendrogram.
     */
    void
    PhylogeticBranch::setLengthForDendrogram()
    {
        setIsDendrogram(true);
        setLength(mLengthForDendrogram);
    }
    
    /**
     *  Prints out the state of a branch object.
     */
    void PhylogeticBranch::description()
    {
        for_each(mChildren.begin(), mChildren.end(),
                 CallDescriptionVector<PhylogeticBranch*>());
        std::cout << "Branch: [" << mLabel << "]:" << mLength
        << "(" << mLengthToRoot << ")"
        << "D:(" << mNumberOfNodes << ")"
        << "R:(" << mRowIndex << ")"
        << "Bxy:(" << mXBegin << "," << mYBegin << ")"
        << "Exy:(" << mXEnd << "," << mYEnd << ")"
        << std::endl;
    }
    
    // goshng delete?
    PhylogeticBranch*
    PhylogeticBranch::branchAt(size_t indexOfBranch)
    {
        if (mRowIndex == indexOfBranch) {
            return this;
        }
        
        for (std::vector<PhylogeticBranch*>::iterator i = mChildren.begin();
             i != mChildren.end(); i++) {
            PhylogeticBranch *branch = (*i)->branchAt(indexOfBranch);
            if (branch != nullptr) {
                return branch;
            }
        }
        return nullptr;
    }
    
    std::string
    PhylogeticBranch::makeQuotedLabel() const
    {
        std::istringstream ss(mLabel);
        std::string s;
        std::string v;

        while (std::getline(ss, s, '\'')) {
            v += s;
            if (!ss.eof()) {
                v += "''";
            }
        }
        return v;
    }
    
    /**
     *  Converts the subtree with a branch and its children branches to a
     * Newick tree string.
     *
     *  @param s The Newick tree return string.
     */
    void
    PhylogeticBranch::toNewickString(std::string& s)
    {
        if (mChildren.size() > 0) {
            s.append(1, NewickParserContant::LeftParenthesis);
            
            for (auto &i : mChildren) {
                if (i != *(mChildren.begin())) {
                    s.append(1, NewickParserContant::Comma);
                }
                i->toNewickString(s);
            }
            s.append(1, NewickParserContant::RightParenthesis);
            if (mLabel.size() > 0) {
                if (mQuotedLabel == true) {
                    s.append(1, NewickParserContant::QuotationMark);
                    s.append(makeQuotedLabel());
                    s.append(1, NewickParserContant::QuotationMark);
                } else {
                    s.append(mLabel);
                }
            }
        } else {
            if (mQuotedLabel == true) {
                s.append(1, NewickParserContant::QuotationMark);
                s.append(makeQuotedLabel());
                s.append(1, NewickParserContant::QuotationMark);
            } else {
                s.append(mLabel);
            }
        }
        
        if (mIsDendrogram == false && mLength > 0) {
            s.append(1, NewickParserContant::Colon);
            std::string stringLength = getStringOfLength();
            if (stringLength.length() > 0) {
                s.append(stringLength);
            } else {
                stringLength = std::to_string(mLength);
                s.append(stringLength);
            }
        }
    }
    
    
#pragma mark - Tree
    
    /**
     *  Constructs a phylogenetic tree.
     */
    PhylogeneticTree::PhylogeneticTree()
    : mRoot(nullptr), mNumberOfNodes(0)
    {
#if defined(DEBUG_LOG)
        std::cout << "Tree: created." << std::endl;
#endif
    }
    
    /**
     *  Destructs a phylogenetic tree.
     */
    PhylogeneticTree::~PhylogeneticTree()
    {
        if (mRoot != nullptr) {
            delete mRoot;
        }
#if defined(DEBUG_LOG)
        std::cout << "Tree: deleted." << std::endl;
#endif
    }
    
    /**
     *  Sets the root of a tree.
     *
     *  @param root The root of a tree.
     */
    void PhylogeneticTree::setRoot(PhylogeticBranch *root)
    {
        assert(root != nullptr);
        mRoot = root;
    }
    
    /**
     *  Sets the maximum length from external nodes to the root of a tree.
     */
    void PhylogeneticTree::setMaxLengthToRoot()
    {
        assert(mRoot != nullptr);
        mMaxLengthToRoot = mRoot->getMaxLengthToRoot();
    }
    
    /**
     *  Configures the tree for drawing.
     *
     *  @param xScale The size of horizontal line of a canvas.
     *
     *  @return 0
     */
    int PhylogeneticTree::configure(double xScale)
    {
        assert(mRoot != nullptr);
        mXScale = xScale;
        mListOfBranches.clear();
        mRoot->enumerateBranch(mListOfBranches);
        mRoot->computeLengthToRoot();
        setMaxLengthToRoot();
        mNumberOfNodes = mRoot->getSetNumberOfNodes();
        mRoot->setRowIndex(mNumberOfNodes);
        mRoot->setCoordinates(mMaxLengthToRoot, mXScale);
        mYScale = static_cast<double>(mNumberOfNodes);
        mYBegin = 0;
        mYEnd = 0;
        mXBegin = xScale * NewickParserContant::ScalebarBegin;
        mXEnd = xScale * NewickParserContant::ScalebarEnd;
        mScaleBarSize = mMaxLengthToRoot * NewickParserContant::ScalebarSize;
        return 0;
    }
    
    /**
     *  Prints out the state of a tree object.
     */
    void PhylogeneticTree::description()
    {
        configure();
        std::cout << "Tree: Nodes(" << mNumberOfNodes << ")" << std::endl;
        mRoot->description();
    }
    
    /**
     *  Draws a tree on a text-based screen.
     */
    void PhylogeneticTree::draw()
    {
        configure();
        
        Graphics graphics(mXScale, mYScale + 1);
        
        // Scale bar and the size
        Point begin(mXBegin, mYBegin);
        Point end(mXEnd, mYEnd);
        graphics.drawHorizontalLine(begin, end);
        graphics.drawText(std::to_string(mScaleBarSize), end);
        graphics.drawPoint(begin, NewickParserContant::Pipe);
        graphics.drawPoint(end, NewickParserContant::Pipe);
        
        // Horizontal lines
        for (auto &branch : mListOfBranches) {
            Point begin(branch->getXBegin(), branch->getYBegin());
            Point end(branch->getXEnd(), branch->getYEnd());
            bool isDendrogram = branch->isDendrogram();
            graphics.drawHorizontalLine(begin, end, isDendrogram);
        }
        
        // Vertical lines
        for (auto &branch : mListOfBranches) {
            if (branch->isInternal()) {
                Point begin(branch->getXChild(), branch->getYChildBegin());
                Point end(branch->getXChild(), branch->getYChildEnd());
                graphics.drawVerticalLine(begin, end);
            }
        }
        
        // Internal nodes
        for (auto &branch : mListOfBranches) {
            if (branch->isInternal()) {
                Point end(branch->getXEnd(), branch->getYEnd());
                graphics.drawPoint(end);
            }
        }
        
        // Labels
        for (auto &branch : mListOfBranches) {
            std::string s = branch->getLabel();
            if (s.length() > 0) {
                Point begin(branch->getXEnd(), branch->getYEnd());
                graphics.drawText(branch->getLabel(), begin);
            }
        }
        graphics.print();
    }
    
    /**
     *  Converts the tree to a Newick tree string.
     *
     *  @return The Newick tree string of a phylogenetic tree.
     */
    std::string
    PhylogeneticTree::toNewickString()
    {
        std::string s;
        mRoot->toNewickString(s);
        s.append(1, NewickParserContant::Semicolon);
        return s;
    }
    
#pragma mark - Generator
    
    /**
     *  Generates a phylogenetic tree at random.
     *
     *  @param maxLength        The threshold length from leaves to the root.
     *  @param length           The mean length of an exponential distribution.
     *  @param numberOfChildren The poisson mean.
     *
     *  @return The generated phylogenetic tree.
     */
    PhylogeneticTree*
    PhylogeneticTreeGenerator::generate(double maxLength, double length,
                                        double numberOfChildren)
    {
        mIndex = 1;
        mMaxLength = maxLength;
        mLength = length;
        mNumberOfChildren = numberOfChildren;
        
        std::poisson_distribution<int> poissonDistribution(mNumberOfChildren);
        std::exponential_distribution<double> exponentialDistribution(mLength);

        PhylogeneticTree *t = new PhylogeneticTree;
        t->setRoot(sample(poissonDistribution, exponentialDistribution, 0,true));
        return t;
    }
    
    /**
     *  Generates a subtree.
     *
     *  @param lengthToRoot The length to the root from the parent node.
     *  @param isRoot       The boolean for whether the branch is rooting.
     *
     *  @return The rooting branch of the subtree.
     */
    PhylogeticBranch*
    PhylogeneticTreeGenerator::sample(std::poisson_distribution<int> &poissonDistribution,
                                      std::exponential_distribution<double> &exponentialDistribution,
                                      double lengthToRoot, bool isRoot)
    {
        PhylogeticBranch *branch = new PhylogeticBranch;
        int numberOfChildren = poissonDistribution(mGenerator);
        double length = exponentialDistribution(mGenerator);
        if (isRoot == true && numberOfChildren == 0) {
            numberOfChildren = 1;
        }
        branch->setLength(length);
        branch->setLengthToRoot(length + lengthToRoot);
        if (branch->getLengthToRoot() < mMaxLength || isRoot == true) {
            
            for (int i = 0; i < numberOfChildren; i++) {
                PhylogeticBranch *childBranch = sample(poissonDistribution,
                                                       exponentialDistribution,
                                                       branch->getLengthToRoot());
                branch->addChild(childBranch);
            }
        } else {
            numberOfChildren = 0;
        }
        
        if (numberOfChildren == 0) {
            std::string label = std::to_string(mIndex++);
            branch->setLabel(label);
        }
        return branch;
    }
    
#pragma mark - NewickParser
    
    /**
     *  Constructs a Newick parser.
     *
     *  @param is The input stream.
     */
    NewickParser::NewickParser(std::istream *is)
    : mIn(is)
    {
#if defined(DEBUG_LOG)
        std::cout << "Parser: created." << std::endl;
#endif
    }
    
    /**
     *  Destructs a Newick parser.
     */
    NewickParser::~NewickParser()
    {
#if defined(DEBUG_LOG)
        std::cout << "Parser: deleted." << std::endl;
#endif
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
        bool isLeftParenthesis = nextFirstLeftParenthesis();
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
        mCharacter = mIn->get();
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
    int NewickParser::nextForQuotedLabel()
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
        int firstCharacter = mIn->get();
        int secondCharacter = mIn->get();
        if (firstCharacter == NewickParserContant::QuotationMark &&
            secondCharacter == NewickParserContant::QuotationMark) {
            mCharacter = NewickParserContant::QuotationMark;
            mInput += firstCharacter;
            mInput += secondCharacter;
        } else {
            if (firstCharacter == NewickParserContant::QuotationMark) {
                mCharacter = 0;
            } else {
                mCharacter = firstCharacter;
            }
            mIn->putback(secondCharacter);
            mInput += firstCharacter;
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
    bool NewickParser::nextFirstLeftParenthesis()
    {
        mOperator = scan();
        if (mOperator == Operator::LEFT_PARENTHESIS) {
            return true;
        } else {
            mErrorCode = ErrorCode::noLeftParenthesis;
            return false;
        }
    }
    
    /**
     *  Creates a tree.
     *
     *  @return The tree.
     */
    PhylogeneticTree*
    NewickParser::tree()
    {
        PhylogeneticTree *tree = new PhylogeneticTree;
        
        return tree;
    }
    
    /**
     *  Parses an input stream for a tree.
     *
     *  @return The tree.
     *
     *  tree := descendant_list [ label ] [ ':' length ]
     */
    PhylogeneticTree*
    NewickParser::nextTree()
    {
        bool isInitialized = initialize();
        
        if (mOperator == Operator::END_OF_FILE) {
            return nullptr;
        } else if (isInitialized == false) {
            putback();
            throw ParsingException();
            return nullptr;
        }
        
        PhylogeneticTree *t = tree();
        PhylogeticBranch *root = subtree();
        if (root == nullptr) {
            putback();
            delete t;
            throw ParsingException();
            return nullptr;
        }
        t->setRoot(root);
        if (mOperator != Operator::SEMICOLON) {
            delete t;
            return nullptr;
        }
        return t;
    }
    
    /**
     *  Gets the input.
     *
     *  @return The input string.
     */
    std::string
    NewickParser::getInput() const
    {
        return mInput;
    }
    
    /**
     *  Parses an input stream for a descendants.
     *
     *  @return The descendant list.
     *
     *  descendant_list := '(' subtree { ',' subtree } ')'
     */
    PhylogeticBranch*
    NewickParser::descendants()
    {
        PhylogeticBranch *branch = new PhylogeticBranch;
        
        PhylogeticBranch *childBranch;
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
    
    /**
     *  Parses an input stream for a label.
     *
     *  @param branch The branch object.
     *
     *  @return The branch object if successful, null otherwise.
     */
    PhylogeticBranch*
    NewickParser::label(PhylogeticBranch *branch)
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
    
    /**
     *  Parses an input stream for a subtree.
     *
     *  @return The branch object if successful, null otherwise.
     */
    PhylogeticBranch*
    NewickParser::subtree()
    {
        PhylogeticBranch *branch = nullptr;
        
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
                if (mOperator == Operator::LABEL ||
                    mOperator == Operator::QUOTED_LABEL) {
                    // label of a branch.
                    if (mOperator == Operator::QUOTED_LABEL) {
                        branch->setQuotedLabel(true);
                    }
                    branch = label(branch);
                    if (branch == nullptr) {
                        return nullptr;
                   }  
                }
                break;
            case Operator::LABEL:
            case Operator::QUOTED_LABEL:
                // label of a leaf.
                branch = new PhylogeticBranch;
                if (mOperator == Operator::QUOTED_LABEL) {
                    branch->setQuotedLabel(true);
                }
                branch = label(branch);
                if (branch == nullptr) {
                    return nullptr;
                }
                break;
            default:
                mErrorCode = ErrorCode::subtree;
                return nullptr;
                break;
        }
        
        // Exception: Two labels in a row
        if (mOperator == Operator::LABEL ||
            mOperator == Operator::QUOTED_LABEL) {
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
            assert(mBuffer.size() > 0);
            double length = 0;
            length = std::stod(mBuffer);
            branch->setStringOfLength(mBuffer);
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
    
    /**
     *  Parses an input stream for a token.
     *
     *  @return The operator or the type of a token.
     */
    NewickParser::Operator
    NewickParser::scan()
    {
    INIT:
        reset();
        
        if (mCharacter == EOF) {
            return Operator::END_OF_FILE;
        }
        
        if (mCharacter == NewickParserContant::Space) {
            ignore();
        }
        
        if (mOperator == Operator::COLON) {
            skipSpaces();
            if (!isLeadingNumber(mCharacter)) {
                mErrorCode = ErrorCode::noNumberAfterColon;
                return Operator::UNKNOWN;
            }
            if (mCharacter == NewickParserContant::Period) {
                read();
                goto TRAILING_DIGIT;
            } else {
                goto LEADING_DIGIT;
            }
        }
        
        if (mCharacter == NewickParserContant::QuotationMark) {
            goto ALPHABET_WITH_QUOTATION_MARK;
        }
        
        if (isLabel(mCharacter)) {
            read();
            goto ALPHABET;
        }
        
    START:
        switch (mCharacter) {
                // Requirements: Blanks or tabs may appear anywhere except within unquoted labels or
                // branch_lengths.
            case NewickParserContant::Space:
            case NewickParserContant::Tab:
            case NewickParserContant::LineFeed:
            case NewickParserContant::EndOfLine:
                ignore();
                goto INIT;
                break;
            case NewickParserContant::Comma:
                read();
                return Operator::COMMA;
            case NewickParserContant::LeftParenthesis:
                read();
                return Operator::LEFT_PARENTHESIS;
            case NewickParserContant::RightParenthesis:
                if (mOperator == Operator::LEFT_PARENTHESIS) {
                    mErrorCode = ErrorCode::emptyDescendant;
                    return Operator::RIGHT_PARENTHESIS;
                }
                read();
                return Operator::RIGHT_PARENTHESIS;
            case NewickParserContant::LeftBracket:
                while (mCharacter != NewickParserContant::RightBracket) {
                    read();
                }
                goto START;
            case NewickParserContant::RightBracket:
                read();
                goto INIT;
            case NewickParserContant::Colon:
                read();
                return Operator::COLON;
            case NewickParserContant::Semicolon:
                return Operator::SEMICOLON;
            case EOF:
                return Operator::END_OF_FILE;
                break;
            default:
                throw ParsingException();
                break;
        }
        
    LEADING_DIGIT:
        if (mCharacter == NewickParserContant::Plus ||
            mCharacter == NewickParserContant::Minus) {
            read();
        }
        if (isnumber(mCharacter)) {
            read();
            goto LEADING_DIGIT;
        } else if (mCharacter == NewickParserContant::Period) {
            read();
            goto TRAILING_DIGIT;
        } else if (mCharacter == NewickParserContant::E ||
                   mCharacter == NewickParserContant::e) {
            read();
            goto SCIENTIFIC_DIGIT_SIGN;
        } else {
            return Operator::NUMBER;
        }
    TRAILING_DIGIT:
        if (isnumber(mCharacter)) {
            read();
            goto TRAILING_DIGIT;
        } else if (mCharacter == NewickParserContant::E ||
                   mCharacter == NewickParserContant::e) {
            read();
            goto SCIENTIFIC_DIGIT_SIGN;
        } else {
            return Operator::NUMBER;
        }
        
    SCIENTIFIC_DIGIT_SIGN:
        if (mCharacter == NewickParserContant::Plus ||
            mCharacter == NewickParserContant::Minus) {
            read();
        }
    SCIENTIFIC_DIGIT:
        if (isnumber(mCharacter)) {
            read();
            goto SCIENTIFIC_DIGIT;
        } else {
            return Operator::NUMBER;
        }

    ALPHABET:
        while (isLabel(mCharacter)) {
            // Requirements: Underscore characters in unquoted labels are converted to blanks.
            if (mCharacter == NewickParserContant::Underscore) {
                mCharacter = NewickParserContant::Space;
            }
            read();
        }
        return Operator::LABEL;
        
    ALPHABET_WITH_QUOTATION_MARK:
        checkForQuotedLabel();
        while (mCharacter != 0) {
            nextForQuotedLabel();
        }
        ignore();
        // Trim the string.
        mBuffer = trim_copy(mBuffer);
        return Operator::QUOTED_LABEL;
    }
    
    /**
     *  Parses an input stream for checking whether a character is a label.
     *
     *  @param ch The character.
     *
     *  @return True if the character is part of a label, false otherwise.
     *
     *  Requirements: Unquoted labels may not contain blanks, parentheses, 
     * square brackets, single_quotes, colons, semicolons, or commas.
     */
    bool
    NewickParser::isLabel(int ch)
    {
        if (isspace(ch) || iscntrl(ch)) {
            return false;
        }
        switch (ch) {
            case NewickParserContant::LeftParenthesis:
            case NewickParserContant::RightParenthesis:
            case NewickParserContant::LeftBracket:
            case NewickParserContant::RightBracket:
            case NewickParserContant::QuotationMark:
            case NewickParserContant::Colon:
            case NewickParserContant::Semicolon:
            case NewickParserContant::Comma:
                return false;
                break;
            default:
                return true;
                break;
        }
    }
    
    /**
     *  Skips spaces.
     */
    void NewickParser::skipSpaces()
    {
        while (isspace(mCharacter)) {
            ignore();
        }
    }
    
    
    /**
     *  Parses an input stream for checking whether a character is a first
     * character of a number.
     *
     *  @param ch The character.
     *
     *  @return True if the character can be the first character of a number,
     * false otherwise.
     */
    bool
    NewickParser::isLeadingNumber(int ch)
    {
        assert(!isspace(ch));
        if (isnumber(ch) ||
            ch == NewickParserContant::Period ||
            ch == NewickParserContant::Plus ||
            ch == NewickParserContant::Minus) {
            return true;
        } else {
            return false;
        }
    }
    
    /**
     *  Skips the input stream when an error occurs.
     *
     *  @return 0
     */
    int
    NewickParser::skip()
    {
        if (mIn == nullptr) {
            assert(0);
        } else {
            mCharacter = mIn->get();
            while (mCharacter != NewickParserContant::EndOfLine &&
                   mCharacter != NewickParserContant::Semicolon &&
                   mCharacter != EOF) {
                mCharacter = mIn->get();
                mInput += mCharacter;
            }
            if (mCharacter == NewickParserContant::Semicolon) {
                mOperator = Operator::SEMICOLON;
            } else if (mCharacter == EOF) {
                mOperator = Operator::END_OF_FILE;
            } else if (mCharacter == NewickParserContant::EndOfLine) {
                mOperator = Operator::END_OF_LINE;
            }
        }
        return 0;
    }
    
    // for checking a bug of putback - goshng
    int
    NewickParser::putback()
    {
        if (mIn == nullptr) {
            assert(0);
        } else {
            mIn->putback(mCharacter);
        }
        return 0;
    }

    /**
     *  Prints out error message.
     */
    void NewickParser::printErrorMessage()
    {
        std::cout << "Error: [code " << static_cast<int>(mErrorCode) << "] ";
        const auto it = mErrorCodeTable.find(mErrorCode);
        if (it == mErrorCodeTable.end()) {
            std::cout << "unknown";
        } else {
            std::cout << it->second;
        }
        std::cout << std::endl;
    }
    
#pragma mark - Graphics
    
    /**
     *  Constructs a point.
     *
     *  @param x X coordinate
     *  @param y Y coordinate
     */
    Point::Point(double x, double y)
    : mX(x), mY(y)
    {
        
    }
    
    /**
     *  Constructs a text-based screen.
     *
     *  @param x The horizontal size of a text-based screen.
     *  @param y The vertical size of a text-based screen.
     */
    Graphics::Graphics(size_t x, size_t y)
    : mX(x), mY(y)
    {
        mCoordinate.resize(y);
        for (auto &i : mCoordinate) {
            i.assign(x, NewickParserContant::Space);
        }
    }
    
    /**
     *  Draws a horizontal line in a text-based screen.
     *
     *  @param begin        The beginnig point.
     *  @param end          The ending point.
     *  @param isDendrogram The boolean value of dendrogram.
     */
    void Graphics::drawHorizontalLine(Point begin, Point end, bool isDendrogram)
    {
        
        size_t xBegin = static_cast<size_t>(std::round(begin.mX));
        size_t xEnd = static_cast<size_t>(std::round(end.mX));
        size_t yBegin = static_cast<size_t>(std::round(begin.mY));
//        size_t yEnd = static_cast<size_t>(std::round(end.mY));
        std::string &line = mCoordinate.at(yBegin);
        for (size_t i = 0; i < line.size(); i++) {
            if (xBegin <= i && i <= xEnd) {
                if (isDendrogram) {
                    line.at(i) = NewickParserContant::Tilde;
                } else {
                    line.at(i) = NewickParserContant::Minus;
                }
            }
        }
    }
    
    
    /**
     *  Draws a vertical line in a text-based screen.
     *
     *  @param begin The beginnig point.
     *  @param end   The ending point.
     */
    void Graphics::drawVerticalLine(Point begin, Point end)
    {
        size_t xBegin = static_cast<size_t>(std::round(begin.mX));
//        size_t xEnd = static_cast<size_t>(std::round(end.mX));
        size_t yBegin = static_cast<size_t>(std::round(begin.mY));
        size_t yEnd = static_cast<size_t>(std::round(end.mY));
        
        for (size_t i = 0; i < mCoordinate.size(); i++) {
            if (yBegin <= i && i <= yEnd) {
                mCoordinate.at(i).at(xBegin) = NewickParserContant::Pipe;
            }
            if (yBegin == i) {
                mCoordinate.at(i).at(xBegin) = NewickParserContant::Slash;
            }
            if (yEnd == i) {
                mCoordinate.at(i).at(xBegin) = NewickParserContant::Backslash;
            }
        }
    }
    
    /**
     *  Draws a point in a text-based screen.
     *
     *  @param begin        The beginnig point.
     *  @param end          The ending point.
     *  @param isDendrogram The boolean value of dendrogram.
     */
    void Graphics::drawPoint(Point end, char ch)
    {
        size_t xEnd = static_cast<size_t>(std::round(end.mX));
        size_t yEnd = static_cast<size_t>(std::round(end.mY));
        if (yEnd < mCoordinate.size()) {
            mCoordinate.at(yEnd).at(xEnd) = ch;
        }
    }
    
    /**
     *  Draws a text in a text-based screen.
     *
     *  @param text  The text string.
     *  @param begin The beginning point.
     */
    void Graphics::drawText(std::string text, Point begin)
    {
        size_t xBegin = static_cast<size_t>(begin.mX);
        size_t yBegin = static_cast<size_t>(begin.mY);
        text.insert(0, 1, ' ');
        std::string &line = mCoordinate.at(yBegin);
        line.replace(xBegin, text.size(), text);
    }
    
    /**
     *  Prints out the graphics context.
     */
    void Graphics::print()
    {
        for (auto &i : mCoordinate) {
            std::cout << i << std::endl;
        }
    }
}

















































