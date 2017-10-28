import java.lang.reflect.Field;
import java.util.Optional;
import java.util.stream.Stream;

/**
 * Created by sulvto on 17-10-26.
 */
public class Type {
    public static final int MUL = 42, ADD = 43, SUB = 45, DOT = 46, DIV = 47, LT = 60, ASSIGN = 61, GT = 62,
            ID = 256, OR = 257, AND = 258, NE = 259, DEF = 260, EQ = 261, NEW = 262, CHAR = 263, CALL = 264,
            BLOCK = 265, NL = 266, IF = 267, ELSE = 268, WHILE = 269, PRINT = 270, RETURN = 271,
            STRUCT = 272, STRING = 273, INT = 274, FLOAT = 275, EOF = 276;


    public static String valueOf(int tag) {
        Optional<Field> value = Stream.of(Type.class.getFields()).filter(field -> {
            try {
                return field.getInt(null) == tag;
            } catch (IllegalAccessException e) {
                return false;
            }
        }).findFirst();
        if (value.isPresent()) {
            return value.get().getName();
        } else {

            char[] a = {(char) tag};
            return new String(a);
        }
    }
}
