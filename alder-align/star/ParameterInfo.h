#ifndef PARAMETERSINFO_DEF
#define PARAMETERSINFO_DEF

class ParameterInfoBase {
public:
    string nameString; //string that identifies parameter
    int inputLevel; //where the parameter was defined
    int inputLevelAllowed; //at which inpurt level parameter definition is allowed
    virtual void inputValues(istringstream &streamIn) =0;
    friend std::ostream& operator<< (std::ostream& o, ParameterInfoBase const& b);    
    virtual ~ParameterInfoBase() {};    
protected:
    virtual void printValues(std::ostream& o) const = 0;
};

inline std::ostream& operator<< (std::ostream& o, ParameterInfoBase const& b) {
    b.printValues(o);
    return o;
};

template <class parameterType>
class ParameterInfoScalar : public ParameterInfoBase {   
public:
    parameterType* value;
    vector <parameterType> allowedValues;
    
    ParameterInfoScalar(int inputLevelIn, int inputLevelAllowedIn, string nameStringIn, parameterType* valueIn) {
        nameString=nameStringIn;
        inputLevel=inputLevelIn;
        inputLevelAllowed=inputLevelAllowedIn;
        value=valueIn;
    };
    
    void inputValues(istringstream &streamIn) {
        streamIn >> *value;
    };
    
    ~ParameterInfoScalar() {};
protected:
   virtual void printValues(std::ostream& o) const {
       o << *value;
   };    
    
};

template <class parameterType>
class ParameterInfoVector : public ParameterInfoBase {   
public:
    vector <parameterType> *value;
    vector <parameterType> allowedValues;
    
    ParameterInfoVector(int inputLevelIn, int inputLevelAllowedIn, string nameStringIn, vector <parameterType> *valueIn) {
        nameString=nameStringIn;
        inputLevel=inputLevelIn;
        inputLevelAllowed=inputLevelAllowedIn;
        value=valueIn;
    };
    
    void inputValues(istringstream &streamIn) {
        parameterType parIn;
        (*value).clear();
        while (streamIn >> parIn) {
            (*value).push_back(parIn);
        };
    };
    
    ~ParameterInfoVector() {};
protected:
   virtual void printValues(std::ostream& o) const {
       for (int ii=0; ii < (int) (*value).size(); ii++) o << (*value).at(ii) <<"   ";
   };     
};
#endif
