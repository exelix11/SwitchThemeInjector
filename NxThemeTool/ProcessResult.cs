namespace NxThemeTool
{
    public record ProcessItem(string Message, string? Source);

    public record ProcessResult(List<ProcessItem> Warnings, List<ProcessItem> Errors)
    {
        public void Warn(string Source, string Message) => 
            Warnings.Add(new ProcessItem(Message, Source));
        
        public void Err(string Source, string Message) =>
            Errors.Add(new ProcessItem(Message, Source));

        public ProcessResult() : this([], []) { }
    }
}
