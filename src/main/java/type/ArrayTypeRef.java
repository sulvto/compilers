package type;

import lexer.Token;

/**
 * Created by sulvto on 17-11-1.
 */
public class ArrayTypeRef extends TypeRef {
    private Token length;

    public ArrayTypeRef(TypeRef ref) {
        super(ref.token);
        this.length = null;
    }

    public ArrayTypeRef(TypeRef ref, Token length) {
        super(ref.token);
        this.length = length;
    }
}
