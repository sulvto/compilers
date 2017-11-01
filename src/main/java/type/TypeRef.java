package type;

import lexer.Token;

/**
 * Created by sulvto on 16-12-18.
 */
public abstract class TypeRef {
    public final Token token;

    public TypeRef(Token token) {
        this.token = token;
    }

}
