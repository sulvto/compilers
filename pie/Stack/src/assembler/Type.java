package assembler;

import java.lang.reflect.Field;
import java.util.Optional;
import java.util.stream.Stream;

/**
 * Created by sulvto on 17-10-26.
 */
public class Type {
    public static final int ID = 256, GLOBALS = 257, DEF = 258,
            ARGS = 259, LOCALS = 260, NEWLINE = 261, STRING = 262,
            CHAR = 263, INT = 264, FLOAT = 265, REG = 266, FUNC = 267, EOF = 268;


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
