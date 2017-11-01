package type;

import lexer.Token;

/**
 * Created by sulvto on 16-12-18.
 */
public class StructTypeRef extends TypeRef {
    private Token name;

    public StructTypeRef(Token token, Token name) {
        super(token);
        this.name = name;
    }

    @Override
    public String toString() {
        return "struct " + name.value;
    }
}
