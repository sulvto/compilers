package type;

import lexer.Token;

/**
 * Created by sulvto on 16-12-18.
 */
public class UnionTypeRef extends TypeRef {
    private Token name;

    public UnionTypeRef(Token token, Token name) {
        super(token);
        this.name = name;
    }

    @Override
    public String toString() {
        return "union " + name.value;
    }
}
