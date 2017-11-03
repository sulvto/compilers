package type;

import exception.SemanticError;

/**
 * Created by sulvto on 17-11-2.
 */
public abstract class Type {
    public boolean isArray() {
        return false;
    }

    public Type baseType() {
        throw new SemanticError("#baseType called for undereferable type");
    }

    public boolean isVoid() {
        return false;
    }

    public FunctionType getFunctionType() {
        return (FunctionType) this;
    }
}
