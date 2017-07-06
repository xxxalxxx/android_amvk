package vulkan.melnichuk.al.amvk;

import android.app.NativeActivity;
import android.os.Bundle;
import android.widget.TextView;

public class MainNativeActivity extends NativeActivity {

    static {
        System.loadLibrary("amvk");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        /*setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());*/
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}
