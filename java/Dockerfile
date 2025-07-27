# Use OpenJDK 22 with Maven for building
FROM openjdk:22-jdk-slim as build

# Set working directory
WORKDIR /app

# Copy pom.xml and download dependencies
COPY pom.xml .
RUN apt-get update && apt-get install -y maven && \
    mvn dependency:go-offline -B

# Copy source code and build
COPY src ./src
RUN mvn clean package -DskipTests

# Use OpenJDK 22 for runtime
FROM openjdk:22-jdk-slim

# Set working directory
WORKDIR /app

# Copy the built JAR file
COPY --from=build /app/target/pingpong-server-1.0.0.jar app.jar

# Add health check
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8000/health || exit 1

# Run the application
CMD ["java", "-jar", "app.jar"]
