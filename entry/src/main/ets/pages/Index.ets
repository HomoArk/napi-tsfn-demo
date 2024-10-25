import { hilog } from '@kit.PerformanceAnalysisKit';
import testNapi from 'libentry.so';

@Entry
@Component
struct Index {
  @State delay: number = 0
  @State min: number = 0;
  @State max: number = 0;
  @State jitter: number = 0;

  build() {
    Row() {
      Column() {
        Text("Delay: " + this.delay)
          .margin(10)
        Text("Min: " + this.min)
          .margin(10)
        Text("Max: " + this.max)
          .margin(10)
        Text("Jitter: " + this.jitter)
          .margin(10)
        Button("Register Callback")
          .onClick(() => {
            testNapi.registerSetDelayCb((delay: number, min: number, max: number, jitter: number) => {
              hilog.info(0x0000, 'SpeedTestDemo', 'delay: %{public}d, min: %{public}d, max: %{public}d, jitter: %{public}d', delay, min, max, jitter);
              this.delay = delay;
              this.min = min;
              this.max = max;
              this.jitter = jitter;
            })
          })
          .margin(10)
        Button("Run")
          .onClick(() => {
              testNapi.runSpeedTest();
          }).margin(10)
      }
      .width('100%')
    }
    .height('100%')
  }
}