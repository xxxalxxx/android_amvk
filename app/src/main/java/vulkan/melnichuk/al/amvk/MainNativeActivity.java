package vulkan.melnichuk.al.amvk;

import android.animation.ValueAnimator;
import android.app.NativeActivity;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.TextView;

import static android.view.MotionEvent.ACTION_DOWN;
import static android.view.MotionEvent.ACTION_UP;

public class MainNativeActivity extends NativeActivity {

    static {
        System.loadLibrary("amvk");
    }

    PopupWindow window;
    Handler handler = new Handler();
    ValueAnimator forwardAnimator = ValueAnimator.ofFloat(0, 1);
    ValueAnimator sidewaysAnimator = ValueAnimator.ofFloat(0, 1);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //initWindow();
        /*setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());*/
    }

    public void showUI() {
        handler.post(new Runnable() {
            @Override
            public void run() {
                if (window == null) initWindow();
            }
        });
    }


    public void initWindow() {
        View v = LayoutInflater.from(this).inflate(R.layout.activity_main, null);
        window = new PopupWindow(v, WindowManager.LayoutParams.WRAP_CONTENT, WindowManager.LayoutParams.WRAP_CONTENT);
        ImageButton up = (ImageButton) v.findViewById(R.id.up);
        ImageButton down = (ImageButton) v.findViewById(R.id.down);
        ImageButton left = (ImageButton) v.findViewById(R.id.left);
        ImageButton right = (ImageButton) v.findViewById(R.id.right);

        left.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent motionEvent) {
                switch (motionEvent.getAction()) {
                    case ACTION_DOWN:
                        setSidewaysMovement(true, -1.0f);
                        break;
                    case ACTION_UP:
                        setSidewaysMovement(false, 0.0f);
                        break;
                }
                return false;
            }
        });

        right.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent motionEvent) {
                switch (motionEvent.getAction()) {
                    case ACTION_DOWN:
                        setSidewaysMovement(true, 1.0f);
                        break;
                    case ACTION_UP:
                        setSidewaysMovement(false, 0.0f);
                        break;
                }
                return false;
            }
        });

        up.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent motionEvent) {
                switch (motionEvent.getAction()) {
                    case ACTION_DOWN:
                        setForwardMovement(true, 1.0f);
                        break;
                    case ACTION_UP:
                        setForwardMovement(false, 0.0f);
                        break;
                }
                return false;
            }
        });

        down.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent motionEvent) {
                switch (motionEvent.getAction()) {
                    case ACTION_DOWN:
                        setForwardMovement(true, -1.0f);
                        break;
                    case ACTION_UP:
                        setForwardMovement(false, 0.0f);
                        break;
                }
                return false;
            }
        });



        LinearLayout mainLayout = new LinearLayout(this);
        ViewGroup.MarginLayoutParams params = new ViewGroup.MarginLayoutParams(
                WindowManager.LayoutParams.WRAP_CONTENT,
                WindowManager.LayoutParams.WRAP_CONTENT);
        params.setMargins(0, 0, 0, 0);
        setContentView(mainLayout, params);

        // Show our UI over NativeActivity window
        window.showAtLocation(mainLayout, Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL, 0, 0);
        window.update();
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native void setSidewaysMovement(boolean moving, float direction);
    public native void setForwardMovement(boolean moving, float direction);
}
