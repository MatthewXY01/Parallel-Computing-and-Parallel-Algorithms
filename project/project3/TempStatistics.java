import java.io.IOException;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.FloatWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.InputSplit;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.input.FileSplit;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;

public class TempStatistics {
    public static class MaxTempMapper extends Mapper<LongWritable, Text, Text, FloatWritable>{
        String csvSplitBy = ",";
        public void map(LongWritable key, Text value, Context context) throws IOException, InterruptedException{
            InputSplit inputSplit = context.getInputSplit();
            String fileName = ((FileSplit) inputSplit).getPath().getName().split("\\.")[0];
            String[] entry = value.toString().split(csvSplitBy);
            if (entry[1].startsWith("\"Time\"")) return;
            Float temp = Float.parseFloat(entry[2]);
            context.write(new Text(fileName), new FloatWritable(temp));
        }
    }
    public static class MaxTempReducer extends Reducer<Text, FloatWritable, Text, FloatWritable>{
        public void reduce (Text key, Iterable<FloatWritable> values, Context context) throws IOException, InterruptedException {
            Float maxValues = Float.MIN_VALUE;
            Float minValues = Float.MAX_VALUE;
            for(FloatWritable value:values){
                maxValues = Math.max(maxValues, value.get());
                minValues = Math.min(minValues, value.get());
            }

            context.write(new Text(key.toString()+"_MAX\t"), new FloatWritable(maxValues));
            context.write(new Text(key.toString()+"_MIN\t"), new FloatWritable(minValues));
        }
    }

    public static void main(String[] args) throws Exception {
        Configuration conf = new Configuration();
        Job job = Job.getInstance(conf, "Temperature Statistics");
        job.setJarByClass(TempStatistics.class);
        job.setMapperClass(MaxTempMapper.class);
//        job.setCombinerCass()
        job.setReducerClass(MaxTempReducer.class);
        job.setOutputKeyClass(Text.class);
        job.setOutputValueClass(FloatWritable.class);
        FileInputFormat.addInputPath(job, new Path(args[0]));
        FileOutputFormat.setOutputPath(job, new Path(args[1]));
        System.exit(job.waitForCompletion(true)? 0 : 1);
    }
}
