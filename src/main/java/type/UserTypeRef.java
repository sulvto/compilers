package type;

import lexer.Token;

/**
 * Created by sulvto on 16-12-18.
 */
public class UserTypeRef extends TypeRef {
    private String name;

    public UserTypeRef(String name) {
        super(null);
        this.name = name;
    }

    public UserTypeRef(Token token, String name) {
        super(token);
        this.name = name;
    }

    @Override
    public String toString() {
        return name;
    }
}
