namespace NxThemeTool
{
    public record ValidationItem(string Message, string? Source);

    public record ValidationResult(List<ValidationItem> Warnings, List<ValidationItem> Errors)
    {
        public void Warn(string Source, string Message) => 
            Warnings.Add(new ValidationItem(Message, Source));
        
        public void Err(string Source, string Message) =>
            Errors.Add(new ValidationItem(Message, Source));

        public ValidationResult() : this([], []) { }
    }
}
