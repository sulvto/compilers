package type;

import ast.Location;

/**
 * Created by sulvto on 16-12-18.
 */
public class UserTypeRef extends TypeRef {
    private String name;

    public UserTypeRef(String name) {
        super(null);
        this.name = name;
    }

    public UserTypeRef(Location location, String name) {
        super(location);
        this.name = name;
    }

    @Override
    public String toString() {
        return name;
    }
}
