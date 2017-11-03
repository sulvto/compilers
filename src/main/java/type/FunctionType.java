package type;

/**
 * Created by sulvto on 17-11-2.
 */
public class FunctionType extends Type {
    protected Type returnType;
    protected ParamTypes paramTypes;

    public Type returnType() {
        return returnType;
    }
}
