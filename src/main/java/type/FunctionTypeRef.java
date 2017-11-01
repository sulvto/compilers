package type;

/**
 * Created by sulvto on 17-10-31.
 */
public class FunctionTypeRef extends TypeRef {
    private TypeRef returnType;

    public FunctionTypeRef(TypeRef returnType) {
        super(returnType.token);
    }
}
